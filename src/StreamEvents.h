#ifndef CMDLINE_MUSICDETECTION_STREAMEVENTS_H
#define CMDLINE_MUSICDETECTION_STREAMEVENTS_H

#include <gnsdk.hpp>
#include <functional>
#include <list>

using namespace gracenote;
using namespace gracenote::musicid_stream;
using metadata::GnResponseAlbums;


template <class T>
class StreamEvents : public IGnMusicIdStreamEvents
{
private:
    std::function<T(GnResponseAlbums&)> callback;
    T result;
    bool has_result_;
    GnError error;
    bool has_error;

public:
    StreamEvents(std::function<T(GnResponseAlbums&)> cb)
     : callback(cb), has_result_(false), has_error(false) {}
    
    bool has_result() {
        return has_result_;
    }
    
    T get_result() {
        return result;
    }
    
    void throw_if_error() {
        if (has_error) {
            throw error;
        }
    }
    
    void StatusEvent(GnStatus Status,
                     gnsdk_uint32_t Complete,
                     gnsdk_size_t Sent,
                     gnsdk_size_t Received,
                     IGnCancellable& Canceller) override
    {
        GNSDK_UNUSED(Status);
        GNSDK_UNUSED(Complete);
        GNSDK_UNUSED(Sent);
        GNSDK_UNUSED(Received);
        GNSDK_UNUSED(Canceller);
    }

    void MusicIdStreamProcessingStatusEvent(
            GnMusicIdStreamProcessingStatus Status,
            IGnCancellable& Canceller
    ) override
    {
        GNSDK_UNUSED(Status);
        GNSDK_UNUSED(Canceller);
    }

    void MusicIdStreamIdentifyingStatusEvent(
            GnMusicIdStreamIdentifyingStatus Status,
            IGnCancellable& Canceller
    ) override
    {
        GNSDK_UNUSED(Status);
        GNSDK_UNUSED(Canceller);
    }

    void MusicIdStreamAlbumResult(
            metadata::GnResponseAlbums& albums,
            IGnCancellable& Canceller
    ) override
    {
        if (albums.ResultCount() > 0) {
            result = callback(albums);
            has_result_ = true;
        }

        GNSDK_UNUSED(Canceller);
    }

    void MusicIdStreamIdentifyCompletedWithError(GnError& e) override
    {
        error = e;
        has_error = true;
    }
};


#endif //CMDLINE_MUSICDETECTION_STREAMEVENTS_H
