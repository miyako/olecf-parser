//
//  main.cpp
//  libolecf-parser
//
//  Created by miyako on 2025/09/09.
//

#include "olecf-parser.h"

/*
#ifdef __APPLE__
#include "cpmap.h"
#define MAX_LENGTH_FOR_ENCODING_NAME (255)
#else
#include <mlang.h>
#endif
*/

/*
static int guess_code_page(const char *data, size_t size) {
    
    int codepage = 1252;
    
#ifdef __APPLE__
    ItemCount count, num;
    
    if(!TECCountAvailableTextEncodings(&count))
    {
        std::vector<TextEncoding> _encodings(count);
        TextEncoding *encodings = &_encodings[0];
        
        TECGetAvailableTextEncodings(encodings, count, &num);
        
        int len = MAX_LENGTH_FOR_ENCODING_NAME;
        
        TECSnifferObjectRef sniffer;
        
        if(!TECCreateSniffer(&sniffer, encodings, num))
        {
            ItemCount numTextEncodings = num;
            ItemCount maxErrs = size;
            ItemCount maxFeatures = size;
            
            std::vector<ItemCount> _numErrsArray(count);
            ItemCount *numErrsArray = &_numErrsArray[0];
            
            std::vector<ItemCount> _numFeaturesArray(count);
            ItemCount *numFeaturesArray = &_numFeaturesArray[0];
            
            OSStatus status = TECSniffTextEncoding(sniffer,
                                                   (ConstTextPtr)data,
                                                   (ByteCount)size,
                                                   encodings,
                                                   numTextEncodings,
                                                   numErrsArray,
                                                   maxErrs,
                                                   numFeaturesArray,
                                                   maxFeatures);
            
            if(status){
                return 1252;//default
            }else{
                
                RegionCode actualRegion;
                TextEncoding actualEncoding;
                ByteCount length;
                
                TextEncoding unicode = CreateTextEncoding(kTextEncodingUnicodeDefault,
                                                          kTextEncodingDefaultVariant,
                                                          kUnicode16BitFormat);
                
                std::vector<char> buf(len);
                
                if(!GetTextEncodingName(
                                        encodings[0],
                                        kTextEncodingFullName,
                                        0,
                                        unicode,
                                        len,
                                        &length,
                                        &actualRegion,
                                        &actualEncoding,
                                        (TextPtr)&buf[0]))
                {
                    CFStringRef name = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar*)&buf[0], (length/2));
                    if(name)
                    {
                        UInt32 cp = TextEncodingNameToWindowsCodepage(name);
                        if(cp > 0) {
                            codepage = cp;
                        }
                        CFRelease(name);
                    }
                    
                }
                
            }
                    
                    TECDisposeSniffer(sniffer);
        }
        
    }
#else
    IMultiLanguage2 *mlang = NULL;
    CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage2, (void **)&mlang);
    
    if(mlang)
    {
        int scores = CP_CODES.getSize();
        std::vector<DetectEncodingInfo> encodings(scores);
        mlang->DetectInputCodepage(MLDETECTCP_NONE, 0, data, (INT *)&size, &encodings[0], &scores);
        
        //no HRESULT?
        INT confidence = 0;
        for(int i = 0; i < scores ; ++i)
        {
            if(encodings[i].nLangID != 0)
            {
                if(confidence < encodings[i].nConfidence){
                    codepage = encodings[i].nCodePage;
                }
            }
        }
        mlang->Release();
    }
#endif
    
    return codepage;
}
*/

#ifdef __APPLE__
CFStringEncoding codepage_to_cfencoding(int cp) {
    
    switch(cp) {
   
        case 1361: return kCFStringEncodingWindowsKoreanJohab;
        case 1258: return kCFStringEncodingWindowsVietnamese;
        case 1257: return kCFStringEncodingWindowsBalticRim;
        case 1256: return kCFStringEncodingWindowsArabic;
        case 1255: return kCFStringEncodingWindowsHebrew;
        case 1254: return kCFStringEncodingWindowsLatin5;
        case 1253: return kCFStringEncodingWindowsGreek;
        case 1252: return kCFStringEncodingWindowsLatin1;
        case 1251: return kCFStringEncodingWindowsCyrillic;
        case 1250: return kCFStringEncodingWindowsLatin2;
        case 950 : return kCFStringEncodingBig5;
        case 949 : return kCFStringEncodingDOSKorean;
        case 936 : return kCFStringEncodingDOSChineseSimplif;
        case 932 : return kCFStringEncodingShiftJIS;
        case 869 : return kCFStringEncodingDOSGreek2;
        case 866 : return kCFStringEncodingDOSRussian;
        case 865 : return kCFStringEncodingDOSNordic;
        case 864 : return kCFStringEncodingDOSArabic;
        case 863 : return kCFStringEncodingDOSCanadianFrench;
        case 862 : return kCFStringEncodingDOSHebrew;
        case 861 : return kCFStringEncodingDOSIcelandic;
        case 860 : return kCFStringEncodingDOSPortuguese;
        case 857 : return kCFStringEncodingDOSTurkish;
        case 855 : return kCFStringEncodingDOSCyrillic;
        case 852 : return kCFStringEncodingDOSLatin2;
        case 851 : return kCFStringEncodingDOSGreek1;
        case 850 : return kCFStringEncodingDOSLatin1;
        case 775 : return kCFStringEncodingDOSBalticRim;
        case 737 : return kCFStringEncodingDOSGreek;
        case 437 : return kCFStringEncodingDOSLatinUS;
        case 37  : return kCFStringEncodingEBCDIC_CP037;
        default:   return 1252; // default
    }
}
#endif

static void utf16_to_utf8(const uint8_t *u16data, size_t u16size, std::string& u8) {
    
#ifdef __APPLE__
    CFStringRef str = CFStringCreateWithCharacters(kCFAllocatorDefault, (const UniChar *)u16data, u16size);
    if(str){
        size_t size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + sizeof(uint8_t);
        std::vector<uint8_t> buf(size+1);
        CFIndex len = 0;
        CFStringGetBytes(str, CFRangeMake(0, CFStringGetLength(str)), kCFStringEncodingUTF8, 0, true, (UInt8 *)buf.data(), buf.size(), &len);
        u8 = (const char *)buf.data();
        CFRelease(str);
    }else{
        u8 = "";
    }
#else
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16data, u16size, NULL, 0, NULL, NULL);
    if(len){
        std::vector<uint8_t> buf(len + 1);
        WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)u16data, u16size, (LPSTR)buf.data(), buf.size(), NULL, NULL));
        u8 = (const char *)buf.data();
    }else{
        u8 = "";
    }
#endif
}

static void ansi_to_utf8(std::string& ansi, std::string& u8, int cp) {
    
#ifdef __APPLE__
    CFDataRef data = CFDataCreate(kCFAllocatorDefault,
                                      reinterpret_cast<const UInt8*>(ansi.data()),
                                      ansi.size());
    CFStringRef str = CFStringCreateFromExternalRepresentation(
                                                               kCFAllocatorDefault,
                                                               data,
                                                               codepage_to_cfencoding(cp));
    CFRelease(data);
    if(str) {
        CFIndex len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str),
                                                        kCFStringEncodingUTF8);
        std::vector<uint8_t> buf(len + 1);
        if (CFStringGetCString(str, (char *)buf.data(), buf.size(), kCFStringEncodingUTF8)) {
            u8 = (const char *)buf.data();
        }else{
            u8 = "";
            std::cerr << "ansi_to_utf8 fail!" << std::endl;
        }
        CFRelease(str);
    }else{
        u8 = "";
    }
#else
    int len = MultiByteToWideChar(cp, 0, (LPCSTR)ansi.data(), ansi.size(), NULL, 0, NULL, NULL);
    if(len){
        std::vector<uint16_t> buf(len + sizeof(uint16_t));
        MultiByteToWideChar(cp, 0, (LPCSTR)ansi.data(), ansi.size(), (LPWSTR)buf.data(), buf.size(), NULL, NULL));
        utf16_to_utf8((const uint8_t *)buf.data(), buf.size(), u8);
    }else{
        u8 = "";
    }
#endif
}

static void usage(void)
{
    fprintf(stderr, "Usage:  olecf-parser -r -i in -o out -\n\n");
    fprintf(stderr, "text extractor for msg documents\n\n");
    fprintf(stderr, " -%c path        : %s\n", 'i' , "document to parse");
    fprintf(stderr, " -%c path        : %s\n", 'o' , "text output (default=stdout)");
    fprintf(stderr, " %c              : %s\n", '-' , "use stdin for input");
    fprintf(stderr, " -%c             : %s\n", 'r' , "raw text output (default=json)");
    fprintf(stderr, " -%c             : %s\n", 'c' , "ansi codepage (default=1252)");
    exit(1);
}

extern OPTARG_T optarg;
extern int optind, opterr, optopt;

#ifdef WIN32
OPTARG_T optarg = 0;
int opterr = 1;
int optind = 1;
int optopt = 0;
int getopt(int argc, OPTARG_T *argv, OPTARG_T opts) {

    static int sp = 1;
    register int c;
    register OPTARG_T cp;
    
    if(sp == 1)
        if(optind >= argc ||
             argv[optind][0] != '-' || argv[optind][1] == '\0')
            return(EOF);
        else if(wcscmp(argv[optind], L"--") == NULL) {
            optind++;
            return(EOF);
        }
    optopt = c = argv[optind][sp];
    if(c == ':' || (cp=wcschr(opts, c)) == NULL) {
        ERR(L": illegal option -- ", c);
        if(argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return('?');
    }
    if(*++cp == ':') {
        if(argv[optind][sp+1] != '\0')
            optarg = &argv[optind++][sp+1];
        else if(++optind >= argc) {
            ERR(L": option requires an argument -- ", c);
            sp = 1;
            return('?');
        } else
            optarg = argv[optind++];
        sp = 1;
    } else {
        if(argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return(c);
}
#define ARGS (OPTARG_T)L"i:o:-rc:h"
#else
#define ARGS "i:o:-rc:h"
#endif

struct Account {
    std::string name;
    std::string address;
};

struct Message {
    std::string subject;
    std::string text;
    std::string html;
    std::string rtf;
    Account sender;
    Account recipient;
    std::string headers;
};

struct Slide {
    std::vector<std::string> text;
};

struct Document {
    std::string type;
    Message message;
    std::vector<Slide> slides;
};

static void utf16le_to_utf_8(std::vector<uint8_t>& buf, std::string& u8) {
    
    buf.insert(buf.end(), 2, 0x00);
    
#ifdef __APPLE__
    CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault,
                                              (const UInt8 *)buf.data(),
                                              buf.size()*sizeof(UniChar), kCFStringEncodingUTF16LE, true);
    if(str)
    {
        CFIndex len = CFStringGetLength(str);
        CFIndex maxSize = CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8) + 1;
        std::vector<uint8_t> _buf(maxSize);
        if (CFStringGetCString(str, (char *)buf.data(), maxSize, kCFStringEncodingUTF8)) {
            u8 = (const char *)buf.data();
        }
        CFRelease(str);
    }
#else
    int len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)buf.data(), buf.size(), NULL, 0, NULL, NULL);
    std::vector<uint8_t> bufu8(len + 1);
    if (WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)buf.data(), buf.size(), (LPSTR)bufu8.data(), len, NULL, NULL)) {
        u8 = (const char*)bufu8.data();
    }
#endif
}

#if defined(_WIN32)
static int create_temp_file_path(std::wstring& path) {
    std::vector<wchar_t>buf(1024);
    if (GetTempPathW((DWORD)buf.size(), buf.data()) == 0) return -1;
    if (GetTempFileNameW(buf.data(), L"pff", 0, buf.data()) == 0) return -1;
    path = std::wstring(buf.data());
    return 0;
}
#else
static int create_temp_file_path(std::string& path) {
    const char *tmpdir = getenv("TMPDIR");
    if (!tmpdir) tmpdir = "/tmp";
    std::vector<char>buf(1024);
    snprintf(buf.data(), buf.size(), "pff%sXXXXXX", tmpdir);
    path = std::string(buf.data());
    int fd = mkstemp((char *)path.c_str());
    if (fd == -1) return -1;
    close(fd);
    return 0;
}
#endif

static int process_rtf(std::vector<uint8_t>& buf, std::vector<uint8_t>& rtf) {
    /*
     3.1.1.2 Compressed RTF Header
     https://learn.microsoft.com/en-us/openspecs/exchange_server_protocols/ms-oxrtfcp/742dec57-50e8-460b-9aaf-504816a7c3de
     */
    
    uint32_t pos = 0x0;
    
    if(buf.size() < 0x10) {
        return -1;//no header!
    }
        
    uint32_t COMPSIZE =
        (uint32_t)buf.at(0)
        | ((uint32_t)buf.at(1) << 8)
        | ((uint32_t)buf.at(2) << 16)
        | ((uint32_t)buf.at(3) << 24);

    if(COMPSIZE != (buf.size() - 0x4)) {
        return -1;//corrupt file!
    }
    
    pos += 4;

    uint32_t RAWSIZE =
        (uint32_t)buf.at(4)
        | ((uint32_t)buf.at(5) << 8)
        | ((uint32_t)buf.at(6) << 16)
        | ((uint32_t)buf.at(7) << 24);

    pos += 4;

    uint32_t COMPRESSED =
        (uint32_t)buf.at(8)
        | ((uint32_t)buf.at(9) << 8)
        | ((uint32_t)buf.at(10) << 16)
        | ((uint32_t)buf.at(11) << 24);

    if(COMPRESSED == 0x414c454d) {
        rtf = buf;
        return 0;//not compressed
    }
    
    pos+=4;
        
    if(COMPRESSED == 0x75465a4c) {
        
        std::string DICT = "{\\rtf1\\ansi\\mac\\deff0\\deftab720{\\fonttbl;}{\\f0\\fnil \\froman \\fswiss \\fmodern \\fscript \\fdecor MS Sans SerifSymbolArialTimes New RomanCourier{\\colortbl\\red0\\green0\\blue0\r\n\\par \\pard\\plain\\f0\\fs20\\b\\i\\u\\tab\\tx";
        
        //CRC
        
        pos += 4;
        
        /*
         2.1.3.1.4 Dictionary
         The dictionary conceptually has a write offset,
         a read offset, and an end offset,
         all of which are zero-based unsigned values
         */
        
        uint32_t ReadOffset  = 0;
        uint32_t WriteOffset = DICT.length();//207

        rtf.clear();
        
        /*
         2.2.2.2 Output
         The output stream MUST initially have a length of zero.
         */
        
        size_t end = buf.size();
        
        while(pos < end) {
            
            uint8_t CONTROL = buf.at(pos);
            pos++;
            
            bool bits[8];
            bits[0] = (CONTROL & 0b00000001);
            bits[1] = (CONTROL & 0b00000010);
            bits[2] = (CONTROL & 0b00000100);
            bits[3] = (CONTROL & 0b00001000);
            bits[4] = (CONTROL & 0b00010000);
            bits[5] = (CONTROL & 0b00100000);
            bits[6] = (CONTROL & 0b01000000);
            bits[7] = (CONTROL & 0b10000000);
            uint8_t count0 = 0;
            uint8_t count1 = 0;
            for(size_t i = 0; i < sizeof(bits); i++) {
                if(bits[i]) {
                    count1++;
                }else{
                    count0++;
                }
            }

            for(uint8_t b = 0; b < sizeof(bits); b++) {

                if(bits[b]) {
                    /*
                     2.1.3.1.5 Dictionary Reference
                     A dictionary reference is a 16-bit packed structure
                     Offset (12 bits):
                     This field contains an index from the beginning of the dictionary
                     that indicates where the matched content will start.
                     Length (4 bits):
                     This value indicates the length of the matched content
                     and is 2 bytes less than the actual length of the matched content.
                     */
                    uint8_t u = buf.at(pos);
                    uint8_t l = buf.at(pos+1);
                    pos+=2;
                    uint32_t dictref = (u << 8) + l;
                    uint32_t len = (dictref & 0b0000000000001111) + 2;
                    ReadOffset = (dictref & 0b1111111111110000) >>4 ;
                    
                    std::string ref = DICT.substr(ReadOffset, len);
                    
                    uint32_t copied = len;
                    uint32_t idx = 0;
                    
                    while (copied != 0) {
                        
                        char v = ref.at(idx);
                        idx++;
                        copied--;
                        
                        if(idx == ref.length()){
                            idx=0;//repeat
                        }
                     
                        if(DICT.length() < 4096) {
                            DICT.push_back(v);
                        }else{
                            DICT[WriteOffset] = v;//circular
                        }
                                                
                        if(WriteOffset == 4095) {
                            WriteOffset=0;//circular
                        }else{
                            WriteOffset++;
                        }
                        rtf.insert(rtf.end(), v);
                    }

                }else{
                                            
                    uint8_t v = buf.at(pos);
                    pos++;
                    
                    if(DICT.length() < 4096) {
                        DICT.push_back(v);
                    }else{
                        DICT[WriteOffset] = v;//circular
                    }
                                        
                    if(WriteOffset == 4095) {
                        WriteOffset=0;//circular
                    }else{
                        WriteOffset++;
                    }
                    rtf.insert(rtf.end(), v);
                }
 
                if(rtf.size() == RAWSIZE) {
                    //escape hatch
                    goto end_of_decompression;
                }
            }
        }
        
        end_of_decompression:
              
        return 0;
    }
    return -1;//not rtf
}

static void document_to_json_msg(Document& document, std::string& text, bool rawText) {
    
    if(rawText){
        text = "";
        text += document.message.sender.name;
        text += document.message.sender.address;
        text += document.message.recipient.name;
        text += document.message.recipient.address;
        text += document.message.subject;
        text += document.message.headers;
        text += document.message.text;
    }else{
        Json::Value documentNode(Json::objectValue);
        documentNode["type"] = document.type;
        
        Json::Value messageNode(Json::objectValue);
        messageNode["subject"] = document.message.subject;
        messageNode["text"] = document.message.text;
        messageNode["html"] = document.message.html;
        messageNode["rtf"] = document.message.rtf;
        messageNode["headers"] = document.message.headers;
        
        Json::Value senderNode(Json::objectValue);
        Json::Value recipientNode(Json::objectValue);
        senderNode["name"] = document.message.sender.name;
        senderNode["address"] = document.message.sender.address;
        recipientNode["name"] = document.message.recipient.name;
        recipientNode["address"] = document.message.recipient.address;

        documentNode["sender"] = senderNode;
        documentNode["recipient"] = recipientNode;
        documentNode["message"] = messageNode;
        
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "";
        text = Json::writeString(writer, documentNode);
    }
}

static void document_to_json_ppt(Document& document, std::string& text, bool rawText) {
    
    if(rawText){
        text = "";
        for(const auto &slide : document.slides) {
            for(const auto &t : slide.text) {
                text += t;
            }
        }
    }else{
        Json::Value documentNode(Json::objectValue);
        documentNode["type"] = document.type;
        
        Json::Value slidesNode(Json::arrayValue);
        
        for(const auto &slide : document.slides) {
            Json::Value slideNode(Json::objectValue);
            Json::Value textNode(Json::arrayValue);
            for(const auto &text : slide.text) {
                textNode.append(text);
            }
            slideNode["text"] = textNode;
            slidesNode.append(slideNode);
        }
        documentNode["slides"] = slidesNode;
        
        Json::StreamWriterBuilder writer;
        writer["indentation"] = "";
        text = Json::writeString(writer, documentNode);
    }
}

typedef struct {
    uint16_t recVerInstance; //recVer(4)+recVerInstance(12)
    uint16_t recType;
    uint32_t recLen;
} RecordHeader;

typedef struct {
    uint32_t x;
    uint32_t y;
} PointStruct;

typedef struct {
    uint32_t numer;
    uint32_t denom;
} RatioStruct;

typedef struct {
    //8+8+8+8=32
    RecordHeader rh;
    PointStruct slideSize;
    PointStruct notesSize;
    RatioStruct serverZoom;
    
    //4+4=8
    uint32_t notesMasterPersistIdRef;
    uint32_t handoutMasterPersistIdRef;
    
    //2+2=4
    uint16_t firstSlideNumber;
    uint16_t slideSizeType;
    
    //1+1+1+1=4
    uint8_t fSaveWithFonts;
    uint8_t fOmitTitlePlace;
    uint8_t fRightToLeft;
    uint8_t fShowComments;
} DocumentAtom;

static uint16_t read_u16_le(const uint8_t *buf) { return buf[0] | (buf[1]<<8); }
static uint32_t read_u32_le(const uint8_t *buf) { return buf[0] | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24); }
static uint32_t read_u32_be(const uint8_t *buf) { return buf[0] << 24 | (buf[1]<<16) | (buf[2]<<8) | (buf[3]); }

static RecordHeader read_RecordHeader(const uint8_t *p) {
    
    size_t offset = 0;
    
    RecordHeader rh;
    rh.recVerInstance = read_u16_le(p);
    offset += sizeof(uint16_t);
    rh.recType        = read_u16_le(p + offset);
    offset += sizeof(uint16_t);
    rh.recLen         = read_u32_le(p + offset);
    
    return rh;
}

static PointStruct read_PointStruct(const uint8_t *p) {
    
    size_t offset = 0;
    
    PointStruct ps;
    ps.x = read_u32_le(p);
    offset += sizeof(uint32_t);
    ps.y = read_u32_le(p + offset);
    
    return ps;
}

static RatioStruct read_RatioStruct(const uint8_t *p) {
    
    size_t offset = 0;
    
    RatioStruct rs;
    rs.numer = read_u32_le(p);
    offset += sizeof(uint32_t);
    rs.denom = read_u32_le(p + offset);
    
    return rs;
}

static DocumentAtom read_DocumentAtom(const uint8_t *p) {
    
    size_t offset = 0;
    
    DocumentAtom da;
    da.rh = read_RecordHeader(p);
    offset += sizeof(RecordHeader);
    
    da.slideSize      = read_PointStruct(p + offset);
    offset += sizeof(PointStruct);
    da.notesSize      = read_PointStruct(p + offset);
    offset += sizeof(PointStruct);
    da.serverZoom     = read_RatioStruct(p + offset);
    offset += sizeof(RatioStruct);

    da.notesMasterPersistIdRef     = read_u32_le(p + offset);
    offset += sizeof(uint32_t);
    da.handoutMasterPersistIdRef   = read_u32_le(p + offset);
    offset += sizeof(uint32_t);
 
    da.firstSlideNumber     = read_u16_le(p + offset);
    offset += sizeof(uint16_t);
    da.slideSizeType     = read_u16_le(p + offset);
    offset += sizeof(uint16_t);
    
    da.fSaveWithFonts = p[offset];
    offset += sizeof(uint8_t);
    da.fOmitTitlePlace = p[offset];
    offset += sizeof(uint8_t);
    da.fRightToLeft = p[offset];
    offset += sizeof(uint8_t);
    da.fShowComments = p[offset];
    
    return da;
}

void read_ppt(Document& document, const uint8_t *stream, size_t stream_len, int codepage) {
    
    size_t offset = 0;
    
    while (offset + sizeof(RecordHeader) <= stream_len) {
        
        RecordHeader rh;
        rh = read_RecordHeader(stream + offset);
        offset += sizeof(RecordHeader);//+=8

        if(rh.recLen == 0) break;
        
        if (offset + rh.recLen > stream_len) break;

        switch (rh.recType) {
            case 0x0:
                return;
                break;
            case 0x0ff5: //RT_UserEditAtom (UserEditAtom)
//                std::cerr << "UserEditAtom" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x1772: //RT_PersistDirectoryAtom (PersistDirectoryAtom)
//                std::cerr << "PersistDirectoryAtom" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03ee: //RT_Slide
//                std::cerr << "SlideContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03ef: //RT_SlideAtom
//                std::cerr << "SlideAtom" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x040c: //RT_Drawing
//                std::cerr << "DrawingContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x0fc9 ://RT_Handout (HandoutContainer)
//                std::cerr << "HandoutContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03F0 ://RT_Notes (NotesContainer)
//                std::cerr << "NotesContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03f1 : //RT_NotesAtom (NotesAtom)
//                std::cerr << "NotesAtom" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03F8 ://RT_MainMaster (MainMasterContainer)
//                std::cerr << "MainMasterContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x1007 ://ExternalMciMovie (ExMCIMovieContainer)
//                std::cerr << "ExMCIMovieContainer" << "(" << rh.recLen << ")"  << std::endl;
                break;
            case 0x03E8 ://RT_Document (DocumentContainer)
            {
//                std::cerr << "DocumentContainer" << "(" << rh.recLen << ")"  << std::endl;
                                                
                DocumentAtom da;
                da = read_DocumentAtom(stream + offset);//RT_DocumentAtom
                                
                size_t da_offset = offset + sizeof(DocumentAtom);
                size_t pos = sizeof(DocumentAtom);
                
                while (pos + sizeof(RecordHeader) <= rh.recLen) {
                    
                    RecordHeader member;
                    member = read_RecordHeader(stream + da_offset);
                    da_offset += sizeof(RecordHeader);

                    pos += sizeof(RecordHeader);
                    
                    switch (member.recType) {
                        case 0x07d0 :
//                        std::cerr << "\tDocInfoListContainer" << "(" << member.recLen << ")"  << std::endl;
                        break;
                        case 0x03ea :
//                        std::cerr << "\tEndDocumentAtom" << "(" << member.recLen << ")"  << std::endl;
                        break;
                        case 0x0428 :
//                        std::cerr << "\tRoundTripCustomTableStyles12Atom" << "(" << member.recLen << ")"  << std::endl;
                        break;
                            case 0x0401 :
//                            std::cerr << "\tSlideShowDocInfoAtom" << "(" << member.recLen << ")"  << std::endl;
                            break;
                        case 0x0Fd9://HeadersFooters
//                            std::cerr << "\tHeadersFooters" << "(" << member.recLen << ")"  << std::endl;
                            break;
                        case 0x0FF0://SlideListWithText
//                            std::cerr << "\tSlideListWithTextContainer" << "(" << member.recLen << ")"  << std::endl;
                        {
//                            slideIndex++;
//                            std::cerr << "slide #" << slideIndex << std::endl;
                            Slide slide;
                            
                            size_t m_offset = da_offset;
                            size_t m_pos = sizeof(RecordHeader);
                            while (m_pos + sizeof(RecordHeader) <= member.recLen) {
                             
                                RecordHeader sl;
                                sl = read_RecordHeader(stream + m_offset);
                                m_pos += sizeof(RecordHeader);
                                m_offset += sizeof(RecordHeader);
                                
                                switch (sl.recType) {
                                    case 0x03f3 :
//                                        std::cerr << "\t\tSlidePersistAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                    case 0x0f9f :
//                                        std::cerr << "\t\tTextHeaderAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                    case 0x0fa9 :
//                                        std::cerr << "\t\tTextSpecialInfoDefaultAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                    case 0x0fa1 :
//                                        std::cerr << "\t\tStyleTextPropAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                    case 0x0faa :
//                                        std::cerr << "\t\tTextSpecialInfoAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                    case 0x0fa8 :
                                    {
//                                        std::cerr << "\t\tTextBytesAtom" << "(" << sl.recLen << ")"  << std::endl;
                                        std::string ansi = std::string((const char *)stream + m_offset, sl.recLen);
                                        std::string u8;

                                        ansi_to_utf8(ansi, u8, codepage);
//                                        std::cerr << u8 << std::endl;
//                                        slide.text.push_back(u8);
                                    }
                                        break;
                                    case 0x0fa0 :
//                                        std::cerr << "\t\tTextCharsAtom" << "(" << sl.recLen << ")"  << std::endl;
                                    {
                                        std::string u8;
                                        utf16_to_utf8((const uint8_t *)stream + m_offset, sl.recLen, u8);
                                        slide.text.push_back(u8);
//                                        std::cerr << u8 << std::endl;
                                    }
                                        break;
                                    default:
//                                        std::cerr << sl.recType << "(" << sl.recLen << ")"  << std::endl;
                                        break;
                                        
                                }
                                m_pos += sl.recLen;;
                                m_offset += sl.recLen;
                            }
                            
                            document.slides.push_back(slide);
                        }
                            break;
                        case 0x040B://DrawingGroup
//                            std::cerr << "\tDrawingGroup" << "(" << member.recLen << ")"  << std::endl;
                            break;
                        case 0x03F2://DocumentTextInfoContainer
//                            std::cerr << "\tDocumentTextInfoContainer" << "(" << member.recLen << ")"  << std::endl;
                            break;
                        default:
//                            std::cerr << member.recType << std::endl;
                            break;
                    }
                    
                    da_offset += member.recLen;
                    pos += member.recLen;
                }
                
            }
                break;
            default:
//                std::cerr << rh.recType << "(" << rh.recLen << ")"  << std::endl;
                continue;
                break;
        }
        
        offset += rh.recLen;
    }
}

static void process_root(Document& document,
                           libolecf_item_t *root, int codepage) {
 
    libolecf_error_t *error = NULL;
    size_t utf8_string_size;
    
    Message message;
    Account sender;
    Account recipient;
    
    std::vector<std::string> properties_to_ignore = {
        "Pictures",
        "Current User",
        "EncryptedSummary",
        "_xmlsignatures",
        "_signatures",
        "\005SummaryInformation",
        "\005DocumentSummaryInformation",
        "__substg1.0_65E30102",
        "__substg1.0_65E20102",
        "__substg1.0_80090048",
        "__substg1.0_3FFB0102",
        "__substg1.0_30140102",
        "__substg1.0_0F030102",
        "__substg1.0_0C1D0102",
        "__substg1.0_0C190102",
        "__substg1.0_00520102",
        "__substg1.0_00510102",
        "__substg1.0_004F0102",
        "__substg1.0_00430102",
        "__substg1.0_00410102",
        "__substg1.0_003F0102",
        "__substg1.0_003B0102",
        "__substg1.0_800B0102",
        "__substg1.0_80080102",
        "__substg1.0_800D001F",
        "__substg1.0_800C001F",
        "__substg1.0_8008001F",
        "__substg1.0_8007001F",
        "__substg1.0_8003001F",
        "__substg1.0_8002001F",
        "__substg1.0_8001001F",
        "__substg1.0_4039001F",
        "__substg1.0_4038001F",
        "__substg1.0_4035001F",
        "__substg1.0_4034001F",
        "__substg1.0_4031001F",
        "__substg1.0_4030001F",
        "__substg1.0_4025001F",
        "__substg1.0_4024001F",
        "__substg1.0_4023001F",
        "__substg1.0_4022001F",
        "__substg1.0_300B0102"/*PidTagSearchKey*/,
        "__substg1.0_00710102"/*PidTagConversationIndex*/,
        "__substg1.0_80090102"/*PidTagAddressBookMember*/,
        "__substg1.0_0064001F"/*PidTagSentRepresentingAddressType*/,
        "__substg1.0_001A001F"/*PidTagMessageClass*/,
        "__substg1.0_0070001F"/*PidTagConversationTopic*/,
        "__substg1.0_8004001F"/*PidTagAddressBookFolderPathname*/,
        "__substg1.0_3FFA001F"/*PidTagLastModifierName*/,
        "__substg1.0_0075001F"/*PidTagReceivedByAddressType*/,
        "__substg1.0_0077001F"/*PidTagReceivedRepresentingAddressType*/,
        "__substg1.0_0050001F",
        "__substg1.0_0C1E001F"/*PidTagSenderAddressType*/,
        "__substg1.0_0E05001F",
        "__substg1.0_1015001F"/*PidTagBodyContentId*/,
        "__substg1.0_1035001F"/*PidTagInternetMessageId*/,
        "__substg1.0_800E001F"/*PidTagAddressBookReports*/
       };
    
    std::vector<std::string> sub_properties_to_ignore = {
        "__substg1.0_00020102",
        "__substg1.0_00030102",
        "__substg1.0_00040102",
        "__substg1.0_10010102",
        "__substg1.0_100F0102",
        "__substg1.0_10110102",
        "__substg1.0_101E0102",
        "__substg1.0_100A0102",
        "__substg1.0_10120102",
        "__substg1.0_10090102",
        "__substg1.0_10140102",
        "__substg1.0_10150102",
        "__substg1.0_10060102",
        "__substg1.0_3D010102",
        "__substg1.0_0FF60102",
        "__substg1.0_101C0102",
        "__substg1.0_10170102",
        "__substg1.0_10080102",
        "__substg1.0_10180102",
        "__substg1.0_10040102",
        "__substg1.0_100D0102",
        "__substg1.0_10190102",
        "__substg1.0_0C240102",
        "__substg1.0_3A0C001F",
        "__substg1.0_371D0102",
        "__substg1.0_3712001F",
        "__substg1.0_370E001F",
        "__substg1.0_3707001F",
        "__substg1.0_3704001F",
        "__substg1.0_3703001F",
        "__substg1.0_37010102",
        "__substg1.0_0FF90102",
        "__substg1.0_5FF70102",
        "__substg1.0_5FE5001F",
        "__substg1.0_3A20001F",
        "__substg1.0_39FE001F",
        "__substg1.0_300B0102",
        "__substg1.0_3003001F",
        "__substg1.0_3002001F",
        "__substg1.0_0FFF0102",
        "__substg1.0_0C250102"
    };
    
    int number_of_sub_items = 0;
    if(libolecf_item_get_number_of_sub_items(root, &number_of_sub_items, &error) == 1) {
        for (int i = 0; i < number_of_sub_items; ++i) {
            libolecf_item_t *sub_item = NULL;
            if(libolecf_item_get_sub_item(root, i, &sub_item, &error) == 1) {
                if(libolecf_item_get_utf8_name_size(sub_item, &utf8_string_size, &error) == 1) {
                    std::vector<uint8_t>property_buf(utf8_string_size + 1);
                    if(libolecf_item_get_utf8_name(sub_item, property_buf.data(), property_buf.size(), &error) == 1) {
                        std::string property = (const char *)property_buf.data();
                        uint32_t size = 0;
                        if(libolecf_item_get_size(sub_item, &size, &error) == 1) {
                            
                            if (document.type == "") {
                                if (property.starts_with("__substg1.0_")) {
                                    document.type = "msg";
                                }
                                if (property == "PowerPoint Document") {
                                    document.type = "ppt";
                                }
                            }
                            
                            if(size == 0) {
                                goto read_sub_items;
                            }
                            
                            for (std::string& uninteresting : properties_to_ignore) {
                                if(property == uninteresting) goto read_sub_items;
                            }

                            std::vector<uint8_t>item_value_buf(size);
                            ssize_t len = libolecf_stream_read_buffer(sub_item, item_value_buf.data(), item_value_buf.size(), NULL);
                            
                            if(property == "PowerPoint Document") {
                                if(len != -1) {
                                    read_ppt(document, item_value_buf.data(), item_value_buf.size(), codepage);
                                }
                                goto read_sub_items;
                            }

                            //PidTagTransportMessageHeaders
                            if(property == "__substg1.0_007D001F") {
                                if(len != -1) {
                                    std::string headers;
                                    utf16le_to_utf_8(item_value_buf, headers);
                                    message.headers = headers;
                                }
                                goto read_sub_items;
                            }
                            
                            //SENDER
                            
                            //PidTagSenderName
                            if(property == "__substg1.0_0C1A001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(item_value_buf, name);
                                    sender.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingName
                            if(property == "__substg1.0_0042001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(item_value_buf, name);
                                    sender.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSenderEmailAddress
                            if(property == "__substg1.0_0C1F001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingEmailAddress
                            if(property == "__substg1.0_0065001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }

                            //RECIPIENT
                            
                            //PidTagEmailAddress
                            if(property == "__substg1.0_3003001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedRepresentingEmailAddress
                            if(property == "__substg1.0_0078001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedByEmailAddress
                            if(property == "__substg1.0_0076001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedByName
                            if(property == "__substg1.0_0040001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(item_value_buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedRepresentingName
                            if(property == "__substg1.0_0044001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(item_value_buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }

                            //PidTagDisplayTo
                            if(property == "__substg1.0_0E04001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(item_value_buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }
     
                            //PidTagSubject:PT_UNICODE
                            if(property == "__substg1.0_0037001F") {
                                if(len != -1) {
                                    std::string subject;
                                    utf16le_to_utf_8(item_value_buf, subject);
                                    message.subject = subject;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagSenderSmtpAddress
                            if(property == "__substg1.0_5D01001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagSentRepresentingSmtpAddress
                            if(property == "__substg1.0_5D02001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(item_value_buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }

                            //PidTagNormalizedSubject:PT_UNICODE
                            if(property == "__substg1.0_0E1D001F") {
                                if((len != -1) && (message.subject.length() == 0)) {
                                    std::string subject;
                                    utf16le_to_utf_8(item_value_buf, subject);
                                    message.subject = subject;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagBody:PT_UNICODE
                            if(property == "__substg1.0_1000001F") {
                                if(len != -1) {
                                    std::string text;
                                    utf16le_to_utf_8(item_value_buf, text);
                                    message.text = text;
                                }
                                goto read_sub_items;
                            }
                            //PidTagRtfCompressed:PT_BINARY
                            if(property == "__substg1.0_10090102") {
                                if(len != -1) {
                                    /*
                                     https://learn.microsoft.com/en-us/openspecs/exchange_server_protocols/ms-oxrtfcp/bf6f7e51-4939-4e16-9b44-6bd644e9ae5d
                                     */
                                    std::vector<uint8_t>rtf;
                                    if(process_rtf(item_value_buf, rtf) == 0) {
                                        message.rtf = std::string((const char *)rtf.data(), rtf.size());
                                    }
                                }
                                goto read_sub_items;
                            }
                            //PidTagHtml:PT_BINARY
                            if(property == "__substg1.0_10130102") {
                                if(len != -1) {
                                    message.html = std::string((const char *)item_value_buf.data(), item_value_buf.size());
                                }
                                goto read_sub_items;
                            }
                        }

//                        std::cerr << property << "(" << size << ")" << std::endl;
                        
                    read_sub_items:
                        
                        int number_of_sub_sub_items = 0;
                        if(libolecf_item_get_number_of_sub_items(sub_item, &number_of_sub_sub_items, &error) == 1) {
                            for (int j = 0; j < number_of_sub_sub_items; ++j) {
                                libolecf_item_t *sub_sub_item = NULL;
                                if(libolecf_item_get_sub_item(sub_item, j, &sub_sub_item, &error) == 1) {
                                    if(libolecf_item_get_utf8_name_size(sub_sub_item, &utf8_string_size, &error) == 1) {
                                        std::vector<uint8_t>buf(utf8_string_size + 1);
                                        if(libolecf_item_get_utf8_name(sub_sub_item, buf.data(), buf.size(), &error) == 1) {
                                            std::string property = (const char *)buf.data();

                                            uint32_t size = 0;
                                            if(libolecf_item_get_size(sub_sub_item, &size, &error) == 1) {

                                                if(size == 0) {
                                                    continue;
                                                }
                                                
                                                bool continue_outer = false;
                                                for (std::string& uninteresting : sub_properties_to_ignore) {
                                                    if(property == uninteresting) {
                                                        continue_outer = true;
                                                        break;
                                                    }
                                                }
                                                
                                                if(continue_outer){
                                                    continue;
                                                }
                                                
                                                std::vector<uint8_t>_buf(size);
                                                ssize_t len = libolecf_stream_read_buffer(sub_sub_item, _buf.data(), _buf.size(), NULL);
                                                
                                                //PidTagRecipientDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_5FF6001F") {
                                                    if(len != -1) {
                                                        std::string name;
                                                        utf16le_to_utf_8(_buf, name);
                                                        recipient.name = name;
                                                    }
                                                    continue;
                                                }
                                                //PidTagDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_3001001F") {
                                                    if(len != -1) {
                                                        std::string name;
                                                        utf16le_to_utf_8(_buf, name);
                                                        recipient.name = name;
                                                    }
                                                    continue;
                                                }
                                                
//                                                std::cerr << "\t" << property << "(" << size << ")" << std::endl;

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    message.sender = sender;
    message.recipient = recipient;
    document.message = message;
}

int main(int argc, OPTARG_T argv[]) {
        
    const OPTARG_T input_path  = NULL;
    const OPTARG_T output_path = NULL;
    
#if defined(_WIN32)
        std::wstring temp_input_path;
#else
        std::string  temp_input_path;
#endif
    
    std::vector<unsigned char>msg_data(0);

    int ch;
    std::string text;
    bool rawText = false;
    int codepage = 1252;
    
    while ((ch = getopt(argc, argv, ARGS)) != -1){
        switch (ch){
            case 'i':
                input_path  = optarg;
                break;
            case 'o':
                output_path = optarg;
                break;
            case 'c':
                codepage = atoi(optarg);
                break;
            case '-':
            {
                _fseek(stdin, 0, SEEK_END);
                size_t len = (size_t)_ftell(stdin);
                _fseek(stdin, 0, SEEK_SET);
                msg_data.resize(len);
                fread(msg_data.data(), 1, msg_data.size(), stdin);
                if(!create_temp_file_path(temp_input_path)){
                    FILE *f = _fopen(temp_input_path.c_str(), _wb);
                    if(f) {
                        fwrite(msg_data.data(), 1, msg_data.size(), f);
                        fclose(f);
                    }
                }
            }
                break;
            case 'r':
                rawText = true;
                break;
            case 'h':
            default:
                usage();
                break;
        }
    }

    const OPTARG_T filename = NULL;
    
    if(input_path) {
        filename = input_path;
    }else{
        if(!msg_data.size()) {
            usage();
        }
        filename = temp_input_path.c_str();
    }
        
    if(input_path) {
        filename = input_path;
    }else{
        if(!msg_data.size()) {
            usage();
        }
        filename = temp_input_path.c_str();
    }

    libolecf_file_t *file = NULL;
    libolecf_error_t *error = NULL;
    
    Document document;

    if (libolecf_file_initialize(&file, &error) == 1) {
        if (_libolecf_file_open(file, filename, LIBOLECF_OPEN_READ, &error) == 1) {
            document.type = "";
            libolecf_item_t *root = NULL;
            if (libolecf_file_get_root_item(file, &root, &error) == 1) {
                process_root(document, root, codepage);
                if(document.type == "msg"){
                    document_to_json_msg(document, text, rawText);
                }
                if(document.type == "ppt"){
                    document_to_json_ppt(document, text, rawText);
                }
            }else{
                std::cerr << "Failed to get MSG root item!" << std::endl;
            }
        }else{
            std::cerr << "Failed to load MSG file!" << std::endl;
        }
        libolecf_file_free(&file, &error);
    }
    
    if(temp_input_path.length()) {
        _unlink(temp_input_path.c_str());
    }
    
    if(!output_path) {
        std::cout << text << std::endl;
    }else{
        FILE *f = _fopen(output_path, _wb);
        if(f) {
            fwrite(text.c_str(), 1, text.length(), f);
            fclose(f);
        }
    }
    
    return 0;
}
