from functools import partial
from subprocess import Popen, PIPE, DEVNULL
from datetime import datetime
import json

import fire
import structlog

import gn
import config


def setup_log():
    structlog.configure(processors=[structlog.processors.TimeStamper(),
                                   structlog.processors.JSONRenderer(ensure_ascii=False)])
    logger = structlog.get_logger()
    return lambda event, **details: logger.msg(event, details=details)


log = setup_log()


sampling_frequency = 44100


def ffmpeg_stream(url):
    ffmpeg = Popen(
        ["ffmpeg",
         "-hide_banner",
#         "-rw_timeout", "10000000",  # network timeout of 10 seconds
         "-i", url,
         "-f", "wav",
         "-ar", str(sampling_frequency),
         "-"],  # decode to stdout
        stdin=DEVNULL,  # don't mess up the outer tty after exit
        stdout=PIPE,    # make stdout a stream
        stderr=open("logs/ffmpeg.log", "wb"))
    return ffmpeg.stdout


def read_exactly(stream, size):
    chunk = b""
    bytes_left = size

    while bytes_left > 0:
        new_chunk = stream.read(bytes_left)
        if len(new_chunk) == 0:
            raise EOFError

        chunk += new_chunk
        bytes_left -= len(new_chunk)

    return chunk


def hour_idx(max_):
    return datetime.now().hour % max_


def main(stream_url):
    num_users = 4

    log("initializing gnsdk")
    with open(config.LICENSE_PATH) as f:
        # keep the reference around
        g = gn.init_gracenote(f.read())
    gn.enable_logging("logs/gn.log")

    log("loading users")
    # FIXME: for some reason, after the first load, gnsdk would not save (and create?) new users
    # so one should create users one by one (one per gnsdk manager?)
    users = list(map(lambda i: gn.create_or_load_user(config.CLIENT_ID,
                                                      config.CLIENT_TAG,
                                                      user_id=i),
                     range(num_users)))

    log("creating ffmpeg stream")
    audio_stream = ffmpeg_stream(stream_url)

    log("recognition_start")
    try:

        for audio_chunk in iter(partial(read_exactly, audio_stream, 7 * sampling_frequency * 2 * 2), None):
            u_id = hour_idx(num_users)
            result = gn.recognize(users[u_id], audio_chunk, log)
            if result["success"]:
                result["albums"] = json.loads(result["albums"])["ALBUM_RESPONSE"]
            log("recognition", user_id=u_id, **result)

    except KeyboardInterrupt:
        log("exiting", reason="keyboard_interrupt")

    except EOFError:
        log("exiting", reason="eof")

    except Exception as e:
        log("crashing", traceback=e)


if __name__ == "__main__":
    fire.Fire(main)
