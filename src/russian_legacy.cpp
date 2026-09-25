#include "nicolai/russian_legacy.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>

namespace nicolai {
namespace {

std::uint32_t cp1251_cp(std::uint8_t b) {
    if (b < 0x80) return b;
    if (b >= 0xC0 && b <= 0xDF) return 0x0410 + (b - 0xC0);
    if (b >= 0xE0) return 0x0430 + (b - 0xE0);
    if (b == 0xA8) return 0x0401;
    if (b == 0xB8) return 0x0451;
    if (b == 0xAB) return 0x00AB;
    if (b == 0xBB) return 0x00BB;
    return 0xFFFD;
}

void append_utf8(std::string& s, std::uint32_t cp) {
    if (cp <= 0x7F) s.push_back(static_cast<char>(cp));
    else if (cp <= 0x7FF) {
        s.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        s.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        s.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string cp1251_to_utf8(const std::string& s) {
    std::string out;
    for (unsigned char b : s) append_utf8(out, cp1251_cp(b));
    return out;
}

std::string trim(std::string s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
    return s;
}

bool decode_one(const std::string& s, std::size_t& i, std::uint32_t& cp) {
    if (i >= s.size()) return false;
    const auto c=static_cast<unsigned char>(s[i]); std::size_t n=0;
    if(c<0x80){cp=c;n=1;} else if((c&0xE0)==0xC0){cp=c&0x1F;n=2;}
    else if((c&0xF0)==0xE0){cp=c&0x0F;n=3;} else if((c&0xF8)==0xF0){cp=c&0x07;n=4;}
    else return false;
    if(i+n>s.size()) return false;
    for(std::size_t j=1;j<n;++j){const auto q=static_cast<unsigned char>(s[i+j]); if((q&0xC0)!=0x80)return false; cp=(cp<<6)|(q&0x3F);} i+=n; return true;
}

std::uint32_t lower_cp(std::uint32_t cp){ if(cp>=0x0410&&cp<=0x042F)return cp+0x20; if(cp==0x0401)return 0x0451; if(cp>='A'&&cp<='Z')return cp+32; return cp; }
std::string lower_utf8(const std::string& s){std::string o; for(std::size_t i=0;i<s.size();){std::uint32_t cp=0;if(!decode_one(s,i,cp)){o.push_back(s[i++]);continue;}append_utf8(o,lower_cp(cp));}return o;}

bool is_word_cp(std::uint32_t cp){cp=lower_cp(cp);return (cp>=0x0430&&cp<=0x044f)||cp==0x0451||(cp>='a'&&cp<='z')||(cp>='0'&&cp<='9');}
std::uint32_t cp_at(const std::string&s,std::size_t at){std::size_t i=at;std::uint32_t cp=0;return decode_one(s,i,cp)?cp:0;}
std::uint32_t cp_before(const std::string&s,std::size_t at){if(at==0)return 0;std::size_t i=at-1;while(i>0&&(static_cast<unsigned char>(s[i])&0xC0)==0x80)--i;return cp_at(s,i);}

bool boundary_ok(const std::string&s,std::size_t pos,std::size_t len){const auto a=cp_before(s,pos), b=pos+len<s.size()?cp_at(s,pos+len):0;return !is_word_cp(a)&&!is_word_cp(b);}

std::string legacy_markup_to_utf8(std::string cp1251) {
    // Remove trailing switches/comments used by the legacy dictionary.
    const auto sw=cp1251.find(" /i"); if(sw!=std::string::npos) cp1251.resize(sw);
    cp1251=trim(cp1251);
    if(!cp1251.empty() && (cp1251.front()=='<' || cp1251.front()=='[')) cp1251.erase(cp1251.begin());
    if(!cp1251.empty() && (cp1251.back()=='>' || cp1251.back()==']')) cp1251.pop_back();
    std::string out;
    for(std::size_t i=0;i<cp1251.size();){
        if(i+1<cp1251.size() && cp1251[i]=='#' && cp1251[i+1]=='#'){out.push_back(' ');i+=2;continue;}
        const unsigned char b=static_cast<unsigned char>(cp1251[i]);
        if(b=='<' || b=='`') { append_utf8(out,0x0301); ++i; continue; }
        append_utf8(out,cp1251_cp(b)); ++i;
    }
    return out;
}

std::string apply_multi(std::string s,const std::vector<std::pair<std::string,std::string>>& rules,std::size_t& count){
    for(const auto&kv:rules){
        if(kv.first.empty()) continue;
        std::size_t search_from=0;
        while(search_from<s.size()){
            const auto low=lower_utf8(s);
            auto pos=low.find(kv.first,search_from);
            if(pos==std::string::npos) break;
            if(!boundary_ok(low,pos,kv.first.size())){search_from=pos+1;continue;}
            const auto replacement_low=lower_utf8(kv.second);
            if(replacement_low==kv.first){search_from=pos+kv.first.size();continue;}
            s.replace(pos,kv.first.size(),kv.second);
            ++count;
            search_from=pos+kv.second.size();
        }
    }
    return s;
}

std::string apply_single_exceptions(const std::string&s,const LegacyExceptionDictionary& d,std::size_t&count){
    std::string out;
    for(std::size_t i=0;i<s.size();){
        const auto start=i;std::uint32_t cp=0;std::size_t j=i;if(!decode_one(s,j,cp)){out.push_back(s[i++]);continue;}
        if(!is_word_cp(cp) || (cp>='0'&&cp<='9')){out.append(s,start,j-start);i=j;continue;}
        std::size_t end=j;
        while(end<s.size()){std::size_t q=end;std::uint32_t z=0;if(!decode_one(s,q,z)||!is_word_cp(z)||(z>='0'&&z<='9'))break;end=q;}
        const auto word=s.substr(start,end-start);const auto it=d.single_word.find(lower_utf8(word));
        if(it!=d.single_word.end()){out+=it->second;++count;}else out+=word;i=end;
    }
    return out;
}

std::string apply_abbreviations(std::string s,const LegacyAbbreviationDictionary& d,std::size_t&count){
    for(std::size_t i=0;i<s.size();){
        bool hit=false;const auto low=lower_utf8(s);
        for(const auto&kv:d.replacements){const auto keylow=lower_utf8(kv.first);if(i+keylow.size()>low.size())continue;if(low.compare(i,keylow.size(),keylow)!=0)continue;if(!boundary_ok(low,i,keylow.size()))continue;s.replace(i,kv.first.size(),kv.second);i+=kv.second.size();++count;hit=true;break;}
        if(!hit){std::size_t j=i;std::uint32_t cp=0;if(!decode_one(s,j,cp)){++i;}else i=j;}
    }
    return s;
}

const char* one(int n){static const char* a[]={"ноль","один","два","три","четыре","пять","шесть","семь","восемь","девять","десять","одиннадцать","двенадцать","тринадцать","четырнадцать","пятнадцать","шестнадцать","семнадцать","восемнадцать","девятнадцать"};return a[n];}
std::string under1000(int n,bool feminine=false){
    static const char* tens[]={"","","двадцать","тридцать","сорок","пятьдесят","шестьдесят","семьдесят","восемьдесят","девяносто"};
    static const char* hund[]={"","сто","двести","триста","четыреста","пятьсот","шестьсот","семьсот","восемьсот","девятьсот"};
    std::vector<std::string> p;if(n>=100){p.emplace_back(hund[n/100]);n%=100;}if(n>=20){p.emplace_back(tens[n/10]);n%=10;}if(n>0){if(feminine&&n==1)p.emplace_back("одна");else if(feminine&&n==2)p.emplace_back("две");else p.emplace_back(one(n));}
    std::string o;for(auto&x:p){if(!o.empty())o+=' ';o+=x;}return o;
}
std::string integer_words(unsigned long long n){
    if(n==0)return "ноль";if(n>999999999ULL)return {};
    std::vector<std::string> p;int mil=n/1000000;n%=1000000;int th=n/1000;int u=n%1000;
    if(mil){p.push_back(under1000(mil));int x=mil%100;if(x>=11&&x<=14)p.push_back("миллионов");else if(mil%10==1)p.push_back("миллион");else if(mil%10>=2&&mil%10<=4)p.push_back("миллиона");else p.push_back("миллионов");}
    if(th){p.push_back(under1000(th,true));int x=th%100;if(x>=11&&x<=14)p.push_back("тысяч");else if(th%10==1)p.push_back("тысяча");else if(th%10>=2&&th%10<=4)p.push_back("тысячи");else p.push_back("тысяч");}
    if(u)p.push_back(under1000(u));std::string o;for(auto&x:p){if(!o.empty())o+=' ';o+=x;}return o;
}
std::string expand_numbers(const std::string&s,std::size_t&count){std::string o;for(std::size_t i=0;i<s.size();){if(s[i]<'0'||s[i]>'9'){o.push_back(s[i++]);continue;}std::size_t j=i;while(j<s.size()&&s[j]>='0'&&s[j]<='9')++j;auto tok=s.substr(i,j-i);try{auto v=std::stoull(tok);auto w=integer_words(v);if(!w.empty()){o+=w;++count;}else o+=tok;}catch(...){o+=tok;}i=j;}return o;}

} // namespace

LegacyAbbreviationDictionary parse_abb_rus_cp1251(const std::vector<std::uint8_t>& bytes){LegacyAbbreviationDictionary d;std::string text(reinterpret_cast<const char*>(bytes.data()),bytes.size());std::size_t p=0;while(p<=text.size()){auto e=text.find('\n',p);if(e==std::string::npos)e=text.size();auto line=text.substr(p,e-p);if(!line.empty()&&line.back()=='\r')line.pop_back();p=e+1;++d.parsed_lines;auto t=trim(line);if(t.empty()||t.rfind("//",0)==0)continue;auto tab=t.find_first_of("\t ");if(tab==std::string::npos)continue;auto key=trim(t.substr(0,tab));auto val=trim(t.substr(tab));if(key.empty()||val.empty())continue;d.replacements.emplace_back(cp1251_to_utf8(key),legacy_markup_to_utf8(val));}std::sort(d.replacements.begin(),d.replacements.end(),[](auto&a,auto&b){return a.first.size()>b.first.size();});d.entries=d.replacements.size();d.valid=d.entries>0;if(!d.valid)d.error="no_abbreviation_entries";return d;}
LegacyAbbreviationDictionary load_abb_rus_cp1251(const std::filesystem::path&path){std::ifstream f(path,std::ios::binary);if(!f){LegacyAbbreviationDictionary d;d.error="cannot_open_abb_rus";return d;}std::vector<std::uint8_t>b((std::istreambuf_iterator<char>(f)),{});return parse_abb_rus_cp1251(b);}

LegacyExceptionDictionary parse_exc_rus_replacements_cp1251(const std::vector<std::uint8_t>&bytes){LegacyExceptionDictionary d;std::string text(reinterpret_cast<const char*>(bytes.data()),bytes.size());std::size_t p=0;while(p<=text.size()){auto e=text.find('\n',p);if(e==std::string::npos)e=text.size();auto line=text.substr(p,e-p);if(!line.empty()&&line.back()=='\r')line.pop_back();p=e+1;++d.parsed_lines;auto t=trim(line);if(t.empty()||t.rfind("//",0)==0)continue;auto c=t.find(':');if(c==std::string::npos)continue;auto key=cp1251_to_utf8(trim(t.substr(0,c)));auto val=legacy_markup_to_utf8(t.substr(c+1));if(key.empty()||val.empty())continue;auto lk=lower_utf8(key);if(lk.find(' ')!=std::string::npos){d.multiword.emplace_back(lk,val);++d.multiword_entries;}else d.single_word[lk]=val;}std::sort(d.multiword.begin(),d.multiword.end(),[](auto&a,auto&b){return a.first.size()>b.first.size();});d.entries=d.single_word.size()+d.multiword.size();d.valid=d.entries>0;if(!d.valid)d.error="no_exception_entries";return d;}
LegacyExceptionDictionary load_exc_rus_replacements_cp1251(const std::filesystem::path&path){std::ifstream f(path,std::ios::binary);if(!f){LegacyExceptionDictionary d;d.error="cannot_open_exc_rus";return d;}std::vector<std::uint8_t>b((std::istreambuf_iterator<char>(f)),{});return parse_exc_rus_replacements_cp1251(b);}

LegacyNormalizationResult normalize_russian_legacy_text(const std::string&utf8,const LegacyAbbreviationDictionary*abb,const LegacyExceptionDictionary*exc){LegacyNormalizationResult r;if(utf8.empty()){r.error="text_empty";return r;}std::string s=utf8;if(abb&&abb->valid)s=apply_abbreviations(std::move(s),*abb,r.abbreviation_replacements);if(exc&&exc->valid){s=apply_multi(std::move(s),exc->multiword,r.exception_replacements);s=apply_single_exceptions(s,*exc,r.exception_replacements);}s=expand_numbers(s,r.number_replacements);r.normalized_utf8=std::move(s);r.valid=true;return r;}

} // namespace nicolai
