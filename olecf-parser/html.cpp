#include <sstream>
#include "html.h"

//#ifdef _WIN32
//string newLine = "\r\n";
//#else
//string newLine = "\n";
//#endif

//4D text is CR by default
static string newLine = "\r";

std::string RtfReader::html::htmlencode(const std::string & text)
{
    return text;
}

const std::string& RtfReader::html::find_color(int param)
{
    if( (size_t)param >= colors.size() )
        return default_color;
    return colors[ param ].html();
}

const std::string& RtfReader::html::find_font(int param)
{
    fonts_container::iterator it = this->fonts.find( param );
    if (it == this->fonts.end())
    {
        // error, needed font not found
        return default_fontname;
    }
    return it->second;
}

// format::processor
void RtfReader::html::begin()
{
    stream << "<html>";
    stream << "<head>";
    in_head = true;
}

void RtfReader::html::end()
{
    if( in_body )
        stream << "</body>";
    else if ( in_head )
        stream << "</head>";

    stream << "</html>";
}

void RtfReader::html::title(string & title)
{
    stream << "<title>" << htmlencode( title ) << "</title>";
}

void RtfReader::html::keywords(string & keywords)
{
    stream << "<keywords>" << htmlencode(keywords) << "</keywords>";
}

void RtfReader::html::author(const string & author)
{
	// Nothing to do
}

void RtfReader::html::summary(summary_categories cat, int param)
{
	// Nothing to do
}

void RtfReader::html::print(const string &txt)
{
    start_body();
    stream << txt;
}

void RtfReader::html::print_symbol(const string &)
{
    start_body();
}
void RtfReader::html::print_forced_space()
{
    start_body();
    stream << "&nbsp;";
}

int  RtfReader::html::print_unicode(int ch)
{
    start_body();
    if (ch <= 0x7F) {
        stream << static_cast<char>(ch);
    } else if (ch <= 0x7FF) {
        stream << static_cast<char>(0xC0 | ((ch >> 6) & 0x1F))
        << static_cast<char>(0x80 | (ch & 0x3F));
    } else if (ch <= 0xFFFF) {
        stream << static_cast<char>(0xE0 | ((ch >> 12) & 0x0F))
        << static_cast<char>(0x80 | ((ch >> 6) & 0x3F))
        << static_cast<char>(0x80 | (ch & 0x3F));
    } else if (ch <= 0x10FFFF) {
        stream << static_cast<char>(0xF0 | ((ch >> 18) & 0x07))
        << static_cast<char>(0x80 | ((ch >> 12) & 0x3F))
        << static_cast<char>(0x80 | ((ch >> 6) & 0x3F))
        << static_cast<char>(0x80 | (ch & 0x3F));
    }
    return 0;
}

void RtfReader::html::hyperlink(string &url)
{
    start_body();
    stream << "<a href='#'>" << htmlencode( url ) << "</a>";
}

void RtfReader::html::hyperlink_base(const string &)
{
	// Nothing to do
}

void RtfReader::html::print_char(special_characters ch)
{
    start_body();
    switch(ch)
    {
        case bullet:
        case left_quote:
            stream << "\""; break;
        case right_quote:
            stream << "\""; break;
        case left_dbl_quote:
            stream << "\""; break;
        case right_dbl_quote:
            stream << "\""; break;
        case nonbreaking_space:
            stream << "&nbsp;"; break;
        case emdash:
            stream << ""; break;
        case endash:
            stream << ""; break;
        case lessthan:
            stream << "&lt;"; break;
        case greaterthan:
            stream << "&gt;"; break;
        case amp:
            stream << "&amp;"; break;
        case copyright:
            stream << "&copy;"; break;
        case trademark:
            stream << "&trade??;"; break;
        case nonbreaking_hyphen:
            stream << "-"; break;
        case optional_hyphen:
            stream << "-"; break;
        case dbl_quote:
            stream << "\""; break;
        case line_break:
            stream << "<br>"; break;
        case page_break:
            stream << "<br>"; break;
        case paragraph_begin:
            stream << "<p>"; break;
        case center_begin:
            stream << "<center>"; break;
        case right_begin:
            stream << ""; break;
        case justify_begin:
            stream << ""; break;
        case center_end:
            stream << ""; break;
        case right_end:
            stream << ""; break;
        case justify_end:
            stream << ""; break;
        case tab:
            stream << "\t"; break;
        default :
            break;
    }
}

void RtfReader::html::font_smaller_begin()
{
    start_body();
    stream << "<small>";
}

void RtfReader::html::font_smaller_end()
{
    start_body();
    stream << "</small>";
}

void RtfReader::html::table_begin()
{
    start_body();
    stream << "<table>";
}

void RtfReader::html::table_end()
{
    start_body();
    stream << "</table>";
}

void RtfReader::html::table_cell_begin()
{
    start_body();
    stream << "<td>";
}

void RtfReader::html::table_cell_end()
{
    start_body();
    stream << "</td>";
}

void RtfReader::html::table_row_begin()
{
    start_body();
    stream << "<tr>";
}

void RtfReader::html::table_row_end()
{
    start_body();
    stream << "</tr>";
}

void RtfReader::html::image( const char * src )
{
    start_body();
    stream << "<img src='" << src << "'>"; // FIXME : urlencode
}

format::image_mgr * RtfReader::html::image_begin(const format::image_descr & image)
{
    start_body();
    return new format::utils::file_image_mgr( "/tmp/zzzzzzz.bmp" );
}

void RtfReader::html::image_end(format::image_mgr &image)
{
    start_body();
    delete & image;
}

void RtfReader::html::start_body()
{
    if( in_body ) return;
    if( in_head )
        stream << "</head>";
    stream << "<body>";
    in_body = true;
    
    attr_push_style( format::attributes::style::character, 0);
}

// format::utils::attributes_stack
void RtfReader::html::enable_attr(int attr, int param, bool enable)
{
    start_body();
    switch( attr )
    {
    case none:
        break;
    case bold:
        stream << ( enable ? "<b>" : "</b>" );
        break;
    case italic:
        stream << ( enable ? "<b>" : "</b>" );
        break;

    case underline:
    case double_ul:
    case word_ul :
    case thick_ul:
    case wave_ul:
    case dot_ul:
    case dash_ul:
    case dot_dash_ul:
    case dbl_dot_dash_ul:
        stream << ( enable ? "<b>" : "</b>" );
        break;

    case fontsize:
        if( enable )
            stream << "<font size='"<< param <<"'>";
        else
            stream << "</font>";
        break;
    case std_fontsize:
        if( enable )
            stream << "<font size='"<< param <<"'>" ;
        else
            stream << "</font>";
        break;
    case fontface:
        if( enable )
            stream << "<font face='" << find_font( param ).c_str() << "'>" ;
        else
            stream << "</font>";
        break;
    case foreground:
        if( enable )
            stream << "<font color='" << find_color( param ).c_str() << "'>" ;
        else
            stream << "</font>";
        break;
    case background:
        stream << ( enable ? "<span>" : "</span>" );
        break;
    case caps:
        break;
    case smallcaps:
        break;

    case shadow:
    case outline:
    case emboss:
    case engrave:

    case super:
    case sub:
    case strike:
    case dbl_strike:
    case expand:
        break;
    }
}

void RtfReader::html::attr_push(int attr, int param)
{
	// Nothing to do
}

void RtfReader::html::attrstack_push()
{
	// Nothing to do
}

void RtfReader::html::attrstack_drop()
{
	// Nothing to do
}

int  RtfReader::html::attr_pop(int)
{
	return 1;
}

void RtfReader::html::attr_drop_all()
{
	// Nothing to do
}

void RtfReader::html::attr_pop_all()
{
	// Nothing to do
}

void RtfReader::html::attr_pop_dump()
{
	// Nothing to do
}

void RtfReader::html::attr_remove(int * tab, int size)
{
	// Nothing to do
}
void RtfReader::html::attr_push_style(style::types type, int id)
{
	// Nothing to do
}

void RtfReader::html::register_style(const style & style)
{
	// Nothing to do
}

// format::utils::fonts_stack
void RtfReader::html::register_font(int num, const char * fontname, int charset)
{
	// Nothing to do
}

// format::colors
void RtfReader::html::register_color(uchar r, uchar g, uchar b)
{
    colors.push_back( html_color(r,g,b) );
}
