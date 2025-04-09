#pragma once

#include <jaffarCommon/hash.hpp>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/file.hpp>
#include <jaffarCommon/json.hpp>
#include <jaffarCommon/serializers/base.hpp>
#include <jaffarCommon/deserializers/base.hpp>
#include <jaffarCommon/serializers/contiguous.hpp>
#include <jaffarCommon/deserializers/contiguous.hpp>
#include "inputParser.hpp"
#include <SDL.h>
#include <libretro.h>

std::string _cdImageFilePath = "__CDROM_PATH.iso";
std::string _romFilePath;

struct MemoryAreas 
{
	uint8_t* wram;
  uint8_t* vram;
};

struct MemorySizes
{
	size_t wram;
  size_t vram;
};

#define VIDEO_HORIZONTAL_PIXELS 480
#define	VIDEO_VERTICAL_PIXELS 270
#define _AUDIO_MAX_SAMPLE_COUNT 4096

extern "C"
{
    void retro_set_audio_sample(retro_audio_sample_t cb);
    void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb);
    void retro_set_environment(retro_environment_t cb);
    void retro_set_input_poll(retro_input_poll_t cb);
    void retro_set_input_state(retro_input_state_t cb);
    void retro_set_log_printf(retro_log_printf_t cb);
    void retro_set_video_refresh(retro_video_refresh_t cb);
    RETRO_API void *retro_get_memory_data(unsigned id);
    RETRO_API size_t retro_get_memory_size(unsigned id);
    void lr_input_device_set(const uint32_t port_, const uint32_t device_);
    RETRO_API size_t retro_serialize_size(void);
    RETRO_API bool retro_serialize(void *data, size_t size);
    RETRO_API bool retro_unserialize(const void *data, size_t size);
}

/// CD Management Logic Start
#define CDIMAGE_SECTOR_SIZE 2048

std::string _gameData;
uint32_t _currentSector = 0;
uint32_t cd_get_size(void) {  return _gameData.size(); }
uint32_t cd_get_sector_count(void) {  return cd_get_size() / CDIMAGE_SECTOR_SIZE; }
void cd_set_sector(const uint32_t sector_) { _currentSector = sector_; }
void cd_read_sector(void *buf_) {  memcpy(buf_, (uint8_t*)&_gameData.data()[_currentSector * CDIMAGE_SECTOR_SIZE], CDIMAGE_SECTOR_SIZE); }
size_t readSegmentFromCD(void *buf_, const uint64_t address, const size_t size)
{
  uint64_t initialSector = address / CDIMAGE_SECTOR_SIZE;
  uint64_t sectorCount = size / CDIMAGE_SECTOR_SIZE;
  uint64_t lastSectorBytes = size % CDIMAGE_SECTOR_SIZE;

  uint8_t tmpBuf[CDIMAGE_SECTOR_SIZE];
  for (uint64_t i = 0; i < sectorCount; i++)
  {
    cd_set_sector(initialSector + i);
    cd_read_sector(tmpBuf);
    memcpy(&((uint8_t*)buf_)[CDIMAGE_SECTOR_SIZE * i], tmpBuf, CDIMAGE_SECTOR_SIZE);
  }

  if (lastSectorBytes > 0)
  {
    cd_set_sector(initialSector + sectorCount);
    cd_read_sector(tmpBuf);
    memcpy(&((uint8_t*)buf_)[CDIMAGE_SECTOR_SIZE * sectorCount], tmpBuf, lastSectorBytes);
  }

  return size;
}
/// CD Management Logic End

namespace jaffar
{

class EmuInstance;
thread_local EmuInstance* _instance = nullptr;

class EmuInstance
{
  public:

  EmuInstance(const nlohmann::json &config)
  {
    _romFilePath = jaffarCommon::json::getString(config, "Rom File Path");
    _inputParser = std::make_unique<jaffar::InputParser>(config);
  }

  ~EmuInstance() = default;
 
  void advanceState(const jaffar::input_t &input)
  {
    _currentInput = input;
     retro_run();
  }

  inline jaffarCommon::hash::hash_t getStateHash() const
  {
    MetroHash128 hash;
    
    //  Getting RAM pointer and size
    // hash.Update(_memoryAreas.wram, _memorySizes.wram);
    // hash.Update(_memoryAreas.vram, _memorySizes.vram);

    jaffarCommon::hash::hash_t result;
    hash.Finalize(reinterpret_cast<uint8_t *>(&result));
    return result;
  }

  bool initialize()
  {
    _instance = this;
    retro_set_environment(retro_environment_callback);
    retro_set_input_poll(retro_input_poll_callback);
    retro_set_audio_sample_batch(retro_audio_sample_batch_callback);
    retro_set_video_refresh(retro_video_refresh_callback);
    retro_set_input_state(retro_input_state_callback);

    // Reading rom file
    auto status = jaffarCommon::file::loadStringFromFile(_gameData, _romFilePath);
    if (status == false) { fprintf(stderr, "Could not open rom file: %s\n", _romFilePath.c_str()); return false; }

    // Normal way to initialize
    retro_init();
    struct retro_game_info game;
    game.path = _romFilePath.c_str();
    auto loadResult = retro_load_game(&game);
    if (loadResult == false) JAFFAR_THROW_RUNTIME("Could not load game: '%s'\n", _romFilePath.c_str());

    _videoBufferSize = VIDEO_HORIZONTAL_PIXELS * VIDEO_VERTICAL_PIXELS * sizeof(uint32_t);
    _videoBuffer = (uint32_t*) malloc (_videoBufferSize);
    _audioBuffer = (uint16_t*) malloc (sizeof(uint16_t) * _AUDIO_MAX_SAMPLE_COUNT);

    // Getting state size
    _stateSize = retro_serialize_size();

    return true;
  }

  void finalize()
  {
    retro_unload_game();
  }

  void initializeVideoOutput()
  {
    SDL_Init(SDL_INIT_VIDEO);
    _renderWindow = SDL_CreateWindow("Opera",  SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED, VIDEO_HORIZONTAL_PIXELS, VIDEO_VERTICAL_PIXELS, 0);
    _renderer = SDL_CreateRenderer(_renderWindow, -1, SDL_RENDERER_ACCELERATED);
    _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_HORIZONTAL_PIXELS, VIDEO_VERTICAL_PIXELS);
  }

  void finalizeVideoOutput()
  {
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_renderWindow);
    SDL_Quit();
  }

  void enableRendering()
  {
    _renderingEnabled = true;
  }

  void disableRendering()
  {
    _renderingEnabled = false;
  }

  void updateRenderer()
  {
    void *pixels = nullptr;
    int pitch = 0;

    SDL_Rect srcRect  = { 0, 0, VIDEO_HORIZONTAL_PIXELS, VIDEO_VERTICAL_PIXELS };
    SDL_Rect destRect = { 0, 0, VIDEO_HORIZONTAL_PIXELS, VIDEO_VERTICAL_PIXELS };

    if (SDL_LockTexture(_texture, nullptr, &pixels, &pitch) < 0) return;

    memcpy(pixels, _videoBuffer, sizeof(uint32_t) * VIDEO_VERTICAL_PIXELS * VIDEO_HORIZONTAL_PIXELS);
    SDL_UnlockTexture(_texture);
    SDL_RenderClear(_renderer);
    SDL_RenderCopy(_renderer, _texture, &srcRect, &destRect);
    SDL_RenderPresent(_renderer);
  }

  void enableStateBlock(const std::string& block) 
  {
    // enableStateBlockImpl(block);
  }

  void disableStateBlock(const std::string& block)
  {
    //  disableStateBlockImpl(block);
  }

  inline size_t getStateSize() const 
  {
    return _stateSize; 
  }

  inline jaffar::InputParser *getInputParser() const { return _inputParser.get(); }
  
  void serializeState(jaffarCommon::serializer::Base& s) const
  {
    retro_serialize(s.getOutputDataBuffer(), _stateSize);
    s.push(nullptr, _stateSize);
  }

  void deserializeState(jaffarCommon::deserializer::Base& d) 
  {
    retro_unserialize(d.getInputDataBuffer(), _stateSize);
    d.pop(nullptr, _stateSize);
  }

  size_t getVideoBufferSize() const { return _videoBufferSize; }
  uint8_t* getVideoBufferPtr() const { return (uint8_t*)_videoBuffer; }

  MemoryAreas getMemoryAreas() const { return _memoryAreas; }
  MemorySizes getMemorySizes() const { return _memorySizes; }

  // functions
  std::string getCoreName() const { return "Headless PPSSPP"; }

  private:

  static __INLINE__ void RETRO_CALLCONV retro_video_refresh_callback(const void *data, unsigned width, unsigned height, size_t pitch)
  {
    printf("Video %p, w: %u, h: %u, p: %lu\n", data, width, height, pitch);
    size_t checksum = 0;
    for (size_t i = 0; i < height; i++)
     for (size_t j = 0; j < width; j++)
     checksum += ((uint32_t*)data)[i*width + j];
    printf("Video Checksum: 0x%lX\n", checksum);
    
    for (size_t i = 0; i < height; i++)
      memcpy(&_instance->_videoBuffer[i * width], &((uint8_t*)data)[i*pitch], sizeof(uint32_t) * width);
  }

  static __INLINE__ size_t RETRO_CALLCONV retro_audio_sample_batch_callback(const int16_t *data, size_t frames)
  {
    printf("Audio frames: %lu\n", frames);
    size_t checksum = 0;
    for (size_t i = 0; i < frames; i++)
     checksum += ((int16_t*)data)[i];
    printf("Audio Checksum: 0x%lX\n", checksum);
    // memcpy(_instance->_audioBuffer, data, sizeof(int16_t) * frames);
    // _instance->_audioSamples = frames;
    return frames;
  }

  static __INLINE__ void RETRO_CALLCONV retro_input_poll_callback()
  {
    // printf("Libretro Input Poll Callback Called:\n");
  }

  static __INLINE__ bool RETRO_CALLCONV retro_environment_callback(unsigned cmd, void *data)
  {
    // printf("Libretro Environment Callback Called: %u\n", cmd);

    if (cmd == RETRO_ENVIRONMENT_GET_LOG_INTERFACE) { *((retro_log_printf_t*)data) = retro_log_printf_callback; return true; }
    if (cmd == RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL) { return true; }
    if (cmd == RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS) { return true; }
    if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE) { _instance->configHandler((struct retro_variable *)data); return true; }
    if (cmd == RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE) { return true; }
    if (cmd == RETRO_ENVIRONMENT_SET_PIXEL_FORMAT) { *((retro_pixel_format*) data) = RETRO_PIXEL_FORMAT_XRGB8888; return true; }
    if (cmd == RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY) { return true; }
    if (cmd == RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY) { return true; } 
    if (cmd == RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_VARIABLES) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_CONTROLLER_INFO) { return false; }
    if (cmd == RETRO_ENVIRONMENT_GET_INPUT_BITMASKS) { return false; }
    if (cmd == RETRO_ENVIRONMENT_GET_USERNAME) { return false; }
    if (cmd == RETRO_ENVIRONMENT_GET_LANGUAGE) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY) { return false; }
    if (cmd == RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SET_HW_RENDER) { return false; }
    if (cmd == RETRO_ENVIRONMENT_SHUTDOWN) { return false; }
    
    JAFFAR_THROW_LOGIC("Unrecognized environment callback command: %u\n", cmd);

    return false;
  }

  __INLINE__ void configHandler(struct retro_variable *var)
  {
    var->value = nullptr;
    printf("Variable Name: %s / Value: %s\n", var->key, var->value);
    std::string key(var->key);
  }

  static __INLINE__ int16_t RETRO_CALLCONV retro_input_state_callback(unsigned port, unsigned device, unsigned index, unsigned id)
  {
    if (device == RETRO_DEVICE_JOYPAD) switch (id)
    {
      case RETRO_DEVICE_ID_JOYPAD_UP: return _instance->_currentInput.up ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_DOWN: return _instance->_currentInput.down ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_LEFT: return _instance->_currentInput.left ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_RIGHT: return _instance->_currentInput.right ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_L: return _instance->_currentInput.ltrigger ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_R: return _instance->_currentInput.rtrigger ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_SELECT: return _instance->_currentInput.select ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_START: return _instance->_currentInput.start ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_X: return _instance->_currentInput.triangle ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_Y: return _instance->_currentInput.square ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_B: return _instance->_currentInput.cross ? 1 : 0;
      case RETRO_DEVICE_ID_JOYPAD_A: return _instance->_currentInput.circle ? 1 : 0;
      default: return 0;
    }

    if (device == RETRO_DEVICE_ANALOG) switch (id)
    {
        case RETRO_DEVICE_ID_ANALOG_X:
          if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) return _instance->_currentInput.leftAnalogX; 
          if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) return _instance->_currentInput.rightAnalogX; 
          return 0;
  
        case RETRO_DEVICE_ID_ANALOG_Y:
          if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) return _instance->_currentInput.leftAnalogY; 
          if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) return _instance->_currentInput.rightAnalogY; 
          return 0;

        default: return 0;
    }
  
    return 0;
  }

  static __INLINE__ void RETRO_CALLCONV retro_log_printf_callback(enum retro_log_level level, const char *format, ...)
  {
    va_list ap;
    va_start(ap, format);
    printf(format, ap);
    va_end(ap);
  }

  static uint32_t InputGetter(void* inputValue) { return *(uint32_t*)inputValue; }

  // State size
  size_t _stateSize = 0;

  // Holds the current input for when the input state callback is called
  jaffar::input_t _currentInput;

  MemoryAreas _memoryAreas;
  MemorySizes _memorySizes;

  // Dummy storage for state load/save
  uint8_t* _dummyStateData;

  // Input parser instance
  std::unique_ptr<jaffar::InputParser> _inputParser;

  // Rendering stuff
  SDL_Window* _renderWindow;
  SDL_Renderer* _renderer;
  SDL_Texture* _texture;
  uint32_t* _videoBuffer;
  size_t _videoBufferSize;
  size_t _videoPitch;

  bool _renderingEnabled = false;
  uint16_t* _audioBuffer;
  size_t _audioSamples = 0;
};

} // namespace jaffar