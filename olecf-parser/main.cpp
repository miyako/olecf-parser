//
//  main.cpp
//  libolecf-parser
//
//  Created by miyako on 2025/09/09.
//

#include "olecf-parser.h"

static void usage(void)
{
    fprintf(stderr, "Usage:  libolecf-parser -r -i in -o out -\n\n");
    fprintf(stderr, "text extractor for msg documents\n\n");
    fprintf(stderr, " -%c path: %s\n", 'i' , "document to parse");
    fprintf(stderr, " -%c path: %s\n", 'o' , "text output (default=stdout)");
    fprintf(stderr, " %c: %s\n", '-' , "use stdin for input");
    fprintf(stderr, " -%c: %s\n", 'r' , "raw text output (default=json)");

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
#define ARGS (OPTARG_T)L"i:o:-rh"
#else
#define ARGS "i:o:-rh"
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

struct Document {
    std::string type;
    Message message;
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
    buf.at(pos++)
    | (buf.at(pos++) << 8 )
    | (buf.at(pos++) << 16)
    | (buf.at(pos++) << 24);
    
    uint32_t RAWSIZE =
    buf.at(pos++)
    | (buf.at(pos++) << 8 )
    | (buf.at(pos++) << 16)
    | (buf.at(pos++) << 24);

    uint32_t COMPRESSED =
    buf.at(pos++)
    | (buf.at(pos++) << 8 )
    | (buf.at(pos++) << 16)
    | (buf.at(pos++) << 24);

    if(COMPRESSED == 0x414c454d) {
        rtf = buf;
        return 0;//not compressed
    }
    
    pos+=4;
    
    if(COMPRESSED == 0x75465a4c) {
        
        std::string DICT = "{\\rtf1\\ansi\\mac\\deff0\\deftab720{\\fonttbl;}{\\f0\\fnil \\froman \\fswiss \\fmodern \\fscript \\fdecor MS Sans SerifSymbolArialTimes New RomanCourier{\\colortbl\\red0\\green0\\blue0\r\n\\par \\pard\\plain\\f0\\fs20\\b\\i\\u\\tab\\tx";
        
        rtf.clear();
        
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
            uint32_t run_length = (count1*2)+count0;
            uint32_t r = 0;
            for(uint8_t b = 0; b < sizeof(bits); b++) {
                if(bits[b]) {
                    uint8_t u = buf.at(pos);
                    uint8_t l = buf.at(pos+1);
                    pos+=2;
                    r+=2;
                    uint32_t dictref = (u << 8) + l;
                    uint32_t len = (dictref & 0b0000000000001111) + 2;
                    uint32_t off = (dictref & 0b1111111111110000) >>4 ;
                    if(off < DICT.length()){
                      
                        std::string ref = DICT.substr(off, len);
                        DICT  +=ref;
                        rtf.insert(rtf.end(), ref.begin(), ref.end());
                        
                    }else{
//                        std::cerr << "offset" << off << " for length " + DICT.length() << std::endl;
                        continue;
                    }
                    
                }else{
                    
                    if(pos < buf.size()){
                        
                        char v = buf.at(pos);
                        pos+=1;
                        r+=1;
                        DICT  +=v;
                        rtf.insert(rtf.end(), v);
                        
                    }else{
//                        std::cerr << "position" << pos << " for size " + buf.size() << std::endl;
                        continue;
                    }
                }
                if((r >= run_length) | (pos >= end)) {
                    break;
                }
            }
        }
        
        return 0;
        
//        if(RAWSIZE == rtf.size()) {
//            
//        }else{
//            return -1;//bad length
//        }
        
    }
    return -1;//not rtf
}

static void document_to_json(Document& document, std::string& text, bool rawText) {
    
    if(rawText){
        text = "";
        text += document.message.sender.name;
        text += document.message.sender.address;
        text += document.message.recipient.name;
        text += document.message.recipient.address;
        text += document.message.subject;
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

static void process_root(Document& document,
                           libolecf_item_t *root) {
 
    libolecf_error_t *error = NULL;
    size_t utf8_string_size;
    
    Message message;
    Account sender;
    Account recipient;
    
    std::vector<std::string> properties_to_ignore = {
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
                    std::vector<uint8_t>buf(utf8_string_size + 1);
                    if(libolecf_item_get_utf8_name(sub_item, buf.data(), buf.size(), &error) == 1) {
                        std::string property = (const char *)buf.data();
                        uint32_t size = 0;
                        if(libolecf_item_get_size(sub_item, &size, &error) == 1) {
                            
                            if(size == 0) {
                                goto read_sub_items;
                            }
                            
                            for (std::string& uninteresting : properties_to_ignore) {
                                if(property == uninteresting) goto read_sub_items;
                            }

                            std::vector<uint8_t>buf(size);
                            ssize_t len = libolecf_stream_read_buffer(sub_item, buf.data(), buf.size(), NULL);
                            
                            //PidTagTransportMessageHeaders
                            if(property == "__substg1.0_007D001F") {
                                if(len != -1) {
                                    std::string headers;
                                    utf16le_to_utf_8(buf, headers);
                                    message.headers = headers;
                                }
                                goto read_sub_items;
                            }
                            
                            //SENDER
                            
                            //PidTagSenderName
                            if(property == "__substg1.0_0C1A001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    sender.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingName
                            if(property == "__substg1.0_0042001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    sender.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSenderEmailAddress
                            if(property == "__substg1.0_0C1F001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingEmailAddress
                            if(property == "__substg1.0_0065001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }

                            //RECIPIENT
                            
                            //PidTagEmailAddress
                            if(property == "__substg1.0_3003001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedRepresentingEmailAddress
                            if(property == "__substg1.0_0078001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedByEmailAddress
                            if(property == "__substg1.0_0076001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    recipient.address = address;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedByName
                            if(property == "__substg1.0_0040001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagReceivedRepresentingName
                            if(property == "__substg1.0_0044001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }

                            //PidTagDisplayTo
                            if(property == "__substg1.0_0E04001F") {
                                if(len != -1) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    recipient.name = name;
                                }
                                goto read_sub_items;
                            }
     
                            //PidTagSubject:PT_UNICODE
                            if(property == "__substg1.0_0037001F") {
                                if((len != -1) && (message.subject.length() == 0)) {
                                    std::string subject;
                                    utf16le_to_utf_8(buf, subject);
                                    message.subject = subject;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagSenderSmtpAddress
                            if(property == "__substg1.0_5D01001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagSentRepresentingSmtpAddress
                            if(property == "__substg1.0_5D02001F") {
                                if(len != -1) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    sender.address = address;
                                }
                                goto read_sub_items;
                            }

                            //PidTagNormalizedSubject:PT_UNICODE
                            if(property == "__substg1.0_0E1D001F") {
                                if((len != -1) && (message.subject.length() == 0)) {
                                    std::string subject;
                                    utf16le_to_utf_8(buf, subject);
                                    message.subject = subject;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagBody:PT_UNICODE
                            if(property == "__substg1.0_1000001F") {
                                if(len != -1) {
                                    std::string text;
                                    utf16le_to_utf_8(buf, text);
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
                                    if(process_rtf(buf, rtf) == 0) {
                                        message.rtf = std::string((const char *)rtf.data(), rtf.size());
                                    }
                                }
                                goto read_sub_items;
                            }
                            //PidTagHtml:PT_BINARY
                            if(property == "__substg1.0_10130102") {
                                if(len != -1) {
                                    message.html = std::string((const char *)buf.data(), buf.size());
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
                                                
                                                std::vector<uint8_t>buf(size);
                                                ssize_t len = libolecf_stream_read_buffer(sub_sub_item, buf.data(), buf.size(), NULL);
                                                
                                                //PidTagRecipientDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_5FF6001F") {
                                                    if(len != -1) {
                                                        std::string name;
                                                        utf16le_to_utf_8(buf, name);
                                                        recipient.name = name;
                                                    }
                                                    continue;
                                                }
                                                //PidTagDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_3001001F") {
                                                    if(len != -1) {
                                                        std::string name;
                                                        utf16le_to_utf_8(buf, name);
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
    
    while ((ch = getopt(argc, argv, ARGS)) != -1){
        switch (ch){
            case 'i':
                input_path  = optarg;
                break;
            case 'o':
                output_path = optarg;
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
            document.type = "msg";
            libolecf_item_t *root = NULL;
            if (libolecf_file_get_root_item(file, &root, &error) == 1) {
                process_root(document, root);
                document_to_json(document, text, rawText);
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
