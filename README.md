![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/olecf-parser)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/olecf-parser/total)

### Dependencies and Licensing

* the source code of this CLI tool is licensed under the MIT license.
* see [libolecf](https://github.com/libyal/libolecf/blob/main/COPYING) for the licensing of **libolecf** (LGPL-3.0).
 
# olecf-parser
CLI tool to extract text from MSG, PPT

## msg

1. use `libolecf` to parse `.msg` binary structure
2. decompress `rtf` according to `[MS-OXRTFCP]` algorithm
3. use `librtf` to convert `rtf` to `html`
4. use `libtidy` to convert `rtf` to `txt`
5. return all `3` body formats in `JSON` mode
6. return `txt` in `RAW` mode (original or converted from `rtf` or `html`)

## usage

```
olecf-parser -i example.msg -o example.json

 -i path    : document to parse
 -o path    : text output (default=stdout)
 -          : use stdin for input
 -r         : raw text output (default=json)
```

## output (JSON)

```
{
    "type: "msg",
    "message":
    {
        "sender": {"name": "name", "address": "address"},
        "recipient": {"name": "name", "address": "address"},
        "subject": "subject",
        "text": "text",
        "rtf": "rtf",
        "html": "html",
        "headers": "headers"
    }
}
```

```
{
    "type: "ppt",
    "slides":
    [
        "text": ["text", "text", "text"]
    ]
}
```
