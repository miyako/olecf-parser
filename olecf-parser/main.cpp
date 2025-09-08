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

struct Message {
    std::string subject;
    std::string text;
    std::string html;
    std::string rtf;
    std::string sender;
    std::string recipient;
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

static void document_to_json(Document& document, std::string& text, bool rawText) {
    
    if(rawText){
        text = "";
        text += document.message.sender;
        text += document.message.recipient;
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
        messageNode["sender"] = document.message.sender;
        messageNode["recipient"] = document.message.recipient;
        
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
                            
                            if(property == "__substg1.0_800B0102") {
                                goto read_sub_items;
                            }
                            
                            if(property == "__substg1.0_80080102") {
                                goto read_sub_items;
                            }
                            
                            //PidTagSearchKey
                            if(property == "__substg1.0_300B0102") {
                                goto read_sub_items;
                            }
                            //PidTagConversationIndex
                            if(property == "__substg1.0_00710102") {
                                goto read_sub_items;
                            }
                            //PidTagAddressBookMember
                            if(property == "__substg1.0_80090102") {
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingAddressType
                            if(property == "__substg1.0_0064001F") {
                                goto read_sub_items;
                            }
                            //PidTagMessageClass
                            if(property == "__substg1.0_001A001F") {
                                goto read_sub_items;
                            }
                            //PidTagConversationTopic
                            if(property == "__substg1.0_0070001F") {
                                goto read_sub_items;
                            }
                            //PidTagAddressBookFolderPathname
                            if(property == "__substg1.0_8004001F") {
                                goto read_sub_items;
                            }
                            //PidTagLastModifierName
                            if(property == "__substg1.0_3FFA001F") {
                                goto read_sub_items;
                            }
                            
                            std::vector<uint8_t>buf(size);
                            ssize_t len = libolecf_stream_read_buffer(sub_item, buf.data(), buf.size(), NULL);
                            
                            //PidTagDisplayTo
                            if(property == "__substg1.0_0E04001F") {
                                if((len != -1) &&(message.recipient.length() == 0)) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    message.recipient = name;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagEmailAddress
                            if(property == "__substg1.0_3003001F") {
                                if((len != -1) &&(message.recipient.length() == 0)) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    message.recipient = name;
                                }
                                goto read_sub_items;
                            }
                            
                            //PidTagSentRepresentingName
                            if(property == "__substg1.0_0042001F") {
                                if((len != -1) &&(message.sender.length() == 0)) {
                                    std::string name;
                                    utf16le_to_utf_8(buf, name);
                                    message.sender = name;
                                }
                                goto read_sub_items;
                            }
                            //PidTagSentRepresentingEmailAddress
                            if(property == "__substg1.0_0065001F") {
                                if((len != -1) &&(message.sender.length() == 0)) {
                                    std::string address;
                                    utf16le_to_utf_8(buf, address);
                                    message.sender = address;
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
                                    goto read_sub_items;
                                }
                            }
                            //PidTagRtfCompressed:PT_BINARY
                            if(property == "__substg1.0_10090102") {
                                if(len != -1) {
//                                    message.rtf = (const char *)buf.data();
                                    goto read_sub_items;
                                }
                            }
                            //PidTagHtml:PT_BINARY
                            if(property == "__substg1.0_10130102") {
                                if(len != -1) {
//                                    message.html = (const char *)buf.data();
                                    goto read_sub_items;
                                }
                            }
                        }
                        
                        std::cout << property << "(" << size << ")" << std::endl;
                        
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
                                            
                                            if(property == "__substg1.0_00020102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_00030102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_00040102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10010102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_100F0102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10110102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_101E0102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_100A0102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10120102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10090102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10140102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10150102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_10060102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_3D010102") {
                                                continue;
                                            }
                                            if(property == "__substg1.0_0FF60102") {
                                                continue;
                                            }

                                            uint32_t size = 0;
                                            if(libolecf_item_get_size(sub_sub_item, &size, &error) == 1) {

                                                if(size == 0) {
                                                    continue;
                                                }
                                                
                                                std::vector<uint8_t>buf(size);
                                                ssize_t len = libolecf_stream_read_buffer(sub_sub_item, buf.data(), buf.size(), NULL);
                                                
                                                //PidTagRecipientDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_5FF6001F") {
                                                    if((len != -1) && (message.recipient.length() == 0)) {
                                                        std::string text;
                                                        utf16le_to_utf_8(buf, text);
                                                        message.recipient = text;
                                                    }
                                                    continue;
                                                }
                                                //PidTagDisplayName:PT_UNICODE
                                                if(property == "__substg1.0_3001001F") {
                                                    if((len != -1) && (message.recipient.length() == 0)) {
                                                        std::string text;
                                                        utf16le_to_utf_8(buf, text);
                                                        message.recipient = text;
                                                    }
                                                    continue;
                                                }
                                                
                                                std::cout << "\t" << property << "(" << size << ")" << std::endl;
  
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
