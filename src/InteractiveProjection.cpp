#include "InteractiveProjection.hpp"

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2};

InteractiveProjection::InteractiveProjection(unsigned int const& rate, unsigned long const& frames,
    int const& bits, unsigned int const& stereo_mode)
    :
    rate_(rate),
    frames_(frames),
    bits_(bits),
    stereo_mode_(stereo_mode) {

  continue_listening_.store(false, std::memory_order_relaxed);

  OpenPcmDevice();
  snd_pcm_hw_params_alloca(&params_);
  SetParametersALSA();
}

InteractiveProjection::~InteractiveProjection() {

}

void InteractiveProjection::ShowALSAParameters() {
  int val;

  std::cout << "ALSA library version: " << SND_LIB_VERSION_STR << std::endl;

  std::cout << std::endl << "PCM stream types:" << std::endl;
  for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
    std::cout << " " << snd_pcm_stream_name((snd_pcm_stream_t) val) <<
              std::endl;

  std::cout << std::endl << "PCM access types:" << std::endl;
  for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
    std::cout << " " << snd_pcm_access_name((snd_pcm_access_t) val) <<
              std::endl;

  std::cout << std::endl << "PCM formats:" << std::endl;
  for (val = 0; val <= SND_PCM_FORMAT_LAST; val++) {
    if (snd_pcm_format_name((snd_pcm_format_t) val) != NULL) {
      std::cout << "  " << snd_pcm_format_name((snd_pcm_format_t) val) <<
                " (" <<
                snd_pcm_format_description((snd_pcm_format_t) val) << ")" <<
                std::endl;
    }
  }

  std::cout << std::endl << "PCM subformats:" << std::endl;
  for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++) {
    std::cout << "  " << snd_pcm_subformat_name((snd_pcm_subformat_t) val) <<
              " (" <<
              snd_pcm_subformat_description((snd_pcm_subformat_t) val) << ")" <<
              std::endl;
  }

  std::cout << std::endl << "PCM states:" << std::endl;
  for (val = 0; val <= SND_PCM_STATE_LAST; val++) {
    std::cout << " " << snd_pcm_state_name((snd_pcm_state_t) val) << std::endl;
  }
}

void AlsaControl::OpenPcmDevice() {
  int rc = snd_pcm_open(&handle_, "default", SND_PCM_STREAM_CAPTURE, 0);

  if (rc < 0) {
    std::cout << "ERROR :  unable to open pcm device: " << snd_strerror(rc) <<
              std::endl;
    exit(1);
  }
}

void AlsaControl::SetParametersALSA() {
  snd_pcm_hw_params_any(handle_, params_); // def values
  snd_pcm_hw_params_set_access(handle_, params_,
      SND_PCM_ACCESS_RW_INTERLEAVED
  ); //non interleaved
  snd_pcm_hw_params_set_format(handle_, params_,
      SND_PCM_FORMAT_S16_LE
  ); //16bits little-endian
  snd_pcm_hw_params_set_channels(handle_, params_,
      stereo_mode_
  ); // stereo ou mono


  snd_pcm_hw_params_set_rate_near(handle_, params_, &rate_,
      NULL
  ); // sample rate (freq echantillonage)
  snd_pcm_hw_params_set_period_size_near(handle_, params_,
      &frames_,
      NULL
  ); //frames pour une période

  int rc = snd_pcm_hw_params(handle_, params_);
  if (rc < 0) {
    std::cout << "ERROR - unable to set hw parameters: " << snd_strerror(rc) <<
              std::endl;
    exit(1);
  }

  snd_pcm_hw_params_get_period_size(params_, &period_size_, NULL);
  snd_pcm_hw_params_get_period_time(params_, &time_period_, NULL);
}

void AlsaControl::Listen() {
  if (!continue_listening_.load(std::memory_order_relaxed)) {
    continue_listening_.store(true, std::memory_order_relaxed);
    thread_ = std::async(std::launch::async, &AlsaControl::ThreadListen,
        this, ""
    );
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" <<
              std::endl;
  }
}

void AlsaControl::Listen(std::string filename) {
  if (!continue_listening_.load(std::memory_order_relaxed)) {
    continue_listening_.store(true, std::memory_order_relaxed);
    thread_ = std::async(std::launch::async, &AlsaControl::ThreadListen,
        this, filename
    );
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" <<
              std::endl;
  }
}

void AlsaControl::ListenWithCallback(std::function<void(void*, int)> func) {
  if (!continue_listening_.load(std::memory_order_relaxed)) {
    continue_listening_.store(true, std::memory_order_relaxed);
    thread_ = std::async(std::launch::async,
        &AlsaControl::ThreadListenWithCallback, this, func, ""
    );
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" <<
              std::endl;
  }
}

void AlsaControl::ListenWithCallback(std::function<void(void*, int)> func,
    std::string filename) {
  if (!continue_listening_.load(std::memory_order_relaxed)) {
    continue_listening_.store(true, std::memory_order_relaxed);
    thread_ = std::async(std::launch::async,
        &AlsaControl::ThreadListenWithCallback, this, func,
        filename
    );
  } else {
    std::cout << "ERROR - System is already listening/recording use stop()" <<
              std::endl;
  }
}

void AlsaControl::RecordToFile(std::string filename,
    int const& duration_in_us) {
  if (!continue_listening_.load(std::memory_order_relaxed)) {
    continue_listening_.store(true, std::memory_order_relaxed);
    thread_ = std::async(std::launch::async,
        &AlsaControl::ThreadRecordToFile, this, filename,
        duration_in_us
    );
    thread_.get();
    continue_listening_.store(false, std::memory_order_relaxed);
  } else {
    std::cout << std::endl <<
              "ERROR - System is already listening/recording use stop()";
  }
}

void AlsaControl::Stop() {
  continue_listening_.store(false, std::memory_order_relaxed);
  thread_.get();
}

void AlsaControl::ThreadListen(std::string filename) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  if (filename != "") {
    filename += ".wav";
    f.open(filename, std::ios::binary);
    WriteHeaderWav(f, rate_, static_cast<short>(bits_),
        static_cast<short>(stereo_mode_), 10000);
    // 10000 is an arbitrary constant because we don't know yet
    // the size of the recording
  }

  snd_pcm_uframes_t size = period_size_ * 2 *
      stereo_mode_; /* 2 bytes/sample, 1 channels */
  void* buffer = malloc(size);


  while (continue_listening_.load(std::memory_order_relaxed)) {
    rc = (int) snd_pcm_readi(handle_, buffer, period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0 && filename != "") {
      f.write(static_cast<char*>(buffer), rc * 2 * stereo_mode_);
      nb_ech += rc;
    }
  }

  free(buffer);

  if (filename != "") {
    WriteHeaderWav(f, rate_, static_cast<short>(bits_),
        static_cast<short>(stereo_mode_),
        nb_ech
    );
    f.close();
  }
}

void AlsaControl::ThreadListenWithCallback(
    std::function<void(void*, int)> func, std::string filename) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  if (filename != "") {
    filename += ".wav";
    f.open(filename, std::ios::binary);
    WriteHeaderWav(f, rate_, static_cast<short>(bits_),
        static_cast<short>(stereo_mode_), 10000
    );
  }

  snd_pcm_uframes_t size = period_size_ * 2 *
      stereo_mode_; /* 2 bytes/sample, 1 channels */
  void* buffer = malloc(size);


  while (continue_listening_.load(std::memory_order_relaxed)) {
    rc = (int) snd_pcm_readi(handle_, buffer, period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0 && filename != "") {
      f.write(static_cast<char*>(buffer), rc * 2 * stereo_mode_);
      nb_ech += rc;
    }

    func(buffer, rc);
  }

  free(buffer);

  if (filename != "") {
    WriteHeaderWav(f, rate_, static_cast<short>(bits_),
        static_cast<short>(stereo_mode_), nb_ech
    );
    f.close();
  }
}

void AlsaControl::ThreadRecordToFile(std::string filename,
    int const& duration_in_us) {
  std::ofstream f;
  int rc;
  int nb_ech = 0;

  filename += ".wav";
  f.open(filename, std::ios::binary);
  WriteHeaderWav(f, rate_, static_cast<short>(bits_),
      static_cast<short>(stereo_mode_), 10000);

  // 2 bytes/sample, 1 channels
  snd_pcm_uframes_t size = period_size_ * 2 * stereo_mode_;

  void* buffer = malloc(size);
  long loops = duration_in_us / time_period_;

  while (loops-- > 0) {
    rc = (int) snd_pcm_readi(handle_, buffer, period_size_);
    if (rc == -EPIPE) {
      std::cout << std::endl << "ERROR - overrun occurred";
      snd_pcm_prepare(handle_);
    } else if (rc < 0) {
      std::cout << std::endl << "ERROR - error from read: " << snd_strerror(rc);
    } else if (rc != (int) period_size_) {
      std::cout << std::endl << "ERROR - short read, read " << rc << " frames";
    }

    if (rc > 0) {
      f.write(static_cast<char*> (buffer), rc * 2 * stereo_mode_);
    }

    nb_ech += rc;
  }

  WriteHeaderWav(f, rate_, static_cast<short>(bits_),
      static_cast<short>(stereo_mode_), nb_ech
  );
  f.close();
  free(buffer);
}

void AlsaControl::ForcePeriodSize(int const& value) {
  period_size_ = static_cast<snd_pcm_uframes_t>(value);
}
