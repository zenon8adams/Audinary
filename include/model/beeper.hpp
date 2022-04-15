#ifndef BEEPER_HPP
#define BEEPER_HPP

#include  <cAudio/cAudio.h>
#include <atomic>
#include <functional>
#include <utility>

class Beeper
{
public:
    static Beeper *instance()
    {
        static Beeper _beeper = Beeper();
        return &_beeper;
    }

    void installBeeper(std::string filename)
    {
        beepFile_ = std::move( filename);
    }

    void setEnabled( bool state)
    {
        is_enabled_ = state;
        if( !is_enabled_) audioManager_->stopAllSounds();
    }

    void play()
    {
        auto can_play = is_enabled_  && !beepFile_.empty() && hasStopped();
        if( audioManager_ != nullptr && can_play)
        {
            currentTrack_ = audioManager_->create( "notification", beepFile_.c_str(), true);

            if( currentTrack_ != nullptr)
                currentTrack_->play2d( false);
        }
    }

private:

    std::string beepFile_;
    bool is_enabled_ = false;

    bool hasStopped()
    {
        return currentTrack_ == nullptr || !currentTrack_->isPlaying();
    }

    Beeper()
    {
        cAudio::getLogger()->unRegisterLogReceiver( "Console");
        cAudio::getLogger()->unRegisterLogReceiver( "File");
        audioManager_ = cAudio::createAudioManager( true);
    }

    ~Beeper()
    {
        if( audioManager_ != nullptr)
            cAudio::destroyAudioManager( audioManager_);
    }

    cAudio::IAudioManager *audioManager_{};
    cAudio::IAudioSource *currentTrack_{};
};

#endif //BEEPER_HPP
