![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm%20|%20win-64&color=blue)
[![license](https://img.shields.io/github/license/miyako/olecf-parser)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/olecf-parser/total)

### Dependencies and Licensing

* the source code of this CLI tool is licensed under the MIT license.
* see [libolecf](https://github.com/libyal/libolecf/blob/main/COPYING) for the licensing of **libolecf** (LGPL-3.0).
 
# olecf-parser
CLI tool to extract text from MSG, DOC, PPT

```
text extractor for msg,doc,ppt documents

 -i path        : document to parse
 -o path        : text output (default=stdout)
 -              : use stdin for input
 -r             : raw text output (default=json)
 -c             : ansi codepage (default=1252)
```

## JSON (MSG)

|Property|Level|Type|Description|
|-|-|-|-|
|document|0|||
|document.type|0|Text||
|document.meta|0| Object ||
|document.meta.subject|1| Text ||
|document.meta.headers|1| Text ||
|document.meta.sender|1| Object ||
|document.meta.recipient|1| Object ||
|document.pages|0|Array||
|document.pages[].paragraphs|1|Array||
|document.pages[].paragraphs[].text|2|Text||
|document.pages[].paragraphs[].html|2|Text||
|document.pages[].paragraphs[].rtf|2|Text||

## JSON (PPT)

|Property|Level|Type|Description|
|-|-|-|-|
|document|0|||
|document.type|0|Text||
|document.pages|0|Array|=slides|
|document.pages[].paragraphs|1|Array||
|document.pages[].paragraphs[].text|2|Text||

## JSON (DOC)

|Property|Level|Type|Description|
|-|-|-|-|
|document|0|||
|document.type|0|Text||
|document.meta|0| Object ||
|document.meta.language|1| Text ||
|document.meta.version|1| Text ||
|document.pages|0|Array||
|document.pages[].paragraphs|1|Array||
|document.pages[].paragraphs[].text|2|Text||



