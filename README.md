![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/olecf-parser)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/olecf-parser/total)

### Dependencies and Licensing

* the source code of this CLI tool is licensed under the MIT license.
* see [libolecf](https://github.com/libyal/libolecf/blob/main/COPYING) for the licensing of **libolecf** (LGPL-3.0).
 
# olecf-parser
CLI tool to extract text from MSG, PPT

## msg in raw mode

sometimes an `.msg` file may not contain plain text body. in such cases, the plain text body is produced by converting the `RTF` or `HTML` body. both `lexbor` and `tidy` parsers are included but `tidy` is used when an `HTML` body exists. native API (`NSAttributedString` on macOS and `RichEdit20W` on Windows) is used to convert `RTF` to plain text. `librtf` is included but not used because of limited codepage support (extended characters are lost).   

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
