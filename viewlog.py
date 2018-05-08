import sys
import json
import textwrap
from datetime import datetime

from schema import Schema, Use, Optional

schema = Schema([
      {
         "title_official":{
            "display":str
         },
         "artist":{
            "name_official":{
               "display":str,
               Optional("prefix"):str,
               Optional("given"):str
            },
         },
         "track":{
            Optional("match_duration"):Use(int),
            "title_official":{
               "display":str
            },
            Optional("match_position"):Use(int),
            Optional("matched"):Use(bool),
            Optional("duration"):{
               "value":Use(int),
            }
         }
      }
],
ignore_extra_keys=True)

default_formatter = lambda d: (json.dumps(d,
                                          ensure_ascii=False,
                                          # indent=4
                                          )
                               if d else "")


def sec_to_min_sec(s):
    return "{:02d}:{:02d}".format(s// 60, s % 60)


def format_albums(albums):
    fj = json.loads(json.dumps(albums),
                    object_hook=lambda d: {k.lower():v for k,v in d.items()})
    formatted = []
    for a in fj["album"]:
        t = list(filter(lambda x: "matched" in x,
                        a["track"]))[0]
        # SIDE-EFFECT
        a["track"] = t

        formatted.append("[{dur}] {artist} — {title} {{{alb}}}".format(
            alb=a["title_official"]["display"],
            dur=sec_to_min_sec(int(t["duration"]["value"]) // 1000),
            title=t["title_official"]["display"],
            artist=(t if "artist" in t else a)["artist"]["name_official"]["display"]))

    return ('\n' + textwrap.indent('\n'.join(formatted),
                                             prefix=" "*24) + '\n'
            if len(formatted) > 1
            else formatted[0])  # + "\n" + json.dumps(schema.validate(fj["album"]), ensure_ascii=False, indent=2)


formatters = {
    "recognition": lambda d: format_albums(d["albums"]) if d["success"] else "(no result)"
}


def format_entry(e):
    return "[{time}] {event}{sep}{details}".format(
        time=datetime.fromtimestamp(e["timestamp"]).strftime("%H:%M:%S"),
        event=e["event"],
        sep=": " if e["details"] else "",
        details=formatters.get(e["event"], default_formatter)(e["details"]))


if __name__ == "__main__":
    try:
        for line in sys.stdin:
            print(format_entry(json.loads(line)))
    except KeyboardInterrupt:
        pass
