#ifndef BEEPER_HPP
#define BEEPER_HPP

#if defined(HOUR_NOTIFICATION)
    #include  <cAudio/cAudio.h>
#endif

#include <atomic>
#include <functional>
#include <utility>
#include <fstream>

class Beeper
{
public:

    struct MemAudio
    {
        const char *bptr = nullptr;
        size_t size = 0;
        bool readonly = false;
    };

    static Beeper *instance()
    {
        static Beeper _beeper = Beeper();
        return &_beeper;
    }

    void installBeeper( const std::string& filename)
    {
#if defined(HOUR_NOTIFICATION)
        std::ifstream handle( filename, std::ios::in);
        if( handle)
        {
            handle.seekg (0, std::ios::end);
            size_t bytes = handle.tellg();
            handle.seekg ( 0, std::ios::beg);

            const char *prev = beep_data_.bptr;

            beep_data_.bptr = ( const char *)malloc( bytes);
            if( !beep_data_.bptr)
            {
                beep_data_.bptr = prev;
                handle.close();
                return;
            }

            if( prev)
                free( (char *)prev);

            beep_data_.size = bytes;

            handle.read( ( char *)beep_data_.bptr, beep_data_.size);

            size_t ext = filename.rfind( '.');
            if( ext == std::string::npos)
                return;

            beeper_extension = std::string( filename.cbegin() + ext + 1, filename.cend());
            setEnabled( true);
        }
#endif
    }

    void installBeeper( const char *start, size_t bytes)
    {
#if defined(HOUR_NOTIFICATION)
        if( start == nullptr || bytes == 0)
            return;

        if( beep_data_.bptr)
            free( (char *)beep_data_.bptr);

        beep_data_.bptr = start;
        beep_data_.size = bytes;
        beep_data_.readonly = true;
        setEnabled(true);
#endif
    }

    void setEnabled( bool state)
    {
#if defined(HOUR_NOTIFICATION)
        is_enabled_ = state;
        if( !is_enabled_) audioManager_->stopAllSounds();
#endif
    }

    void play()
    {
#if defined(HOUR_NOTIFICATION)
        auto can_play = is_enabled_  && beep_data_.bptr && hasStopped();
        if( audioManager_ != nullptr && can_play)
        {
            currentTrack_ = audioManager_->createFromMemory( "notification", beep_data_.bptr, beep_data_.size, beeper_extension.c_str());

            if( currentTrack_ != nullptr)
            {
                currentTrack_->play2d( false);
                while( currentTrack_->isPlaying())
                    cAudio::cAudioSleep(10);
            }
        }
#endif
    }

private:

    bool is_enabled_ = false;
    MemAudio beep_data_;

    std::string beeper_extension = "ogg";

    bool hasStopped()
    {
#if defined(HOUR_NOTIFICATION)
        return currentTrack_ == nullptr || !currentTrack_->isPlaying();
#else
        return false;
#endif
    }

    Beeper()
    {
#if defined(HOUR_NOTIFICATION)
        cAudio::getLogger()->unRegisterLogReceiver( "Console");
        cAudio::getLogger()->unRegisterLogReceiver( "File");
        audioManager_ = cAudio::createAudioManager( true);
#endif
    }

    ~Beeper()
    {
#if defined(HOUR_NOTIFICATION)
        if( audioManager_ != nullptr)
            cAudio::destroyAudioManager( audioManager_);

        if( beep_data_.bptr != nullptr && !beep_data_.readonly)
            free( ( char *)beep_data_.bptr);
#endif
    }

#if defined(HOUR_NOTIFICATION)
    cAudio::IAudioManager *audioManager_{};
    cAudio::IAudioSource *currentTrack_{};
#endif
};

#endif //BEEPER_HPP
