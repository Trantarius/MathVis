#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <cassert>

constexpr uint64_t str_hash(const char* cstr,ulong len){
  uint64_t hash=0;
  uint8_t off=0;
  uint64_t idx=0;
  do{
    hash ^= ((uint64_t)cstr[idx])<<off;
    off = (off+13)%57;
  }while(++idx<len);
  return hash;
}

consteval uint64_t operator ""_hash(const char* cstr,ulong len){
  return str_hash(cstr,len);
}

inline uint64_t str_hash(const std::string& str){
  return str_hash(str.c_str(),str.length());
}

class ID{

  struct CStr{
    const char* ptr=nullptr;
    uint64_t len=0;

    void set(const char* p,uint64_t l){
      if(ptr) delete [] ptr;
      char* pt=new char[l+1]{0};
      memcpy(pt,p,l);
      ptr=pt;
      len=l;
    }

    CStr(){}
    CStr(const char* p){
      set(p,strlen(p));
    }
    CStr(const char* p,uint64_t l){
      set(p,l);
    }
    CStr(const CStr& b){*this=b;}
    CStr(CStr&& b){*this=std::move(b);}

    CStr& operator=(const CStr& b){
      set(b.ptr,b.len);
      return *this;
    }
    CStr& operator=(CStr&& b){
      ptr=b.ptr;
      len=b.len;
      b.ptr=nullptr;
      b.len=0;
      return *this;
    }

    bool operator==(const CStr& b) const {
      return len==b.len && !strcmp(ptr,b.ptr);
    }

    ~CStr(){
      if(ptr) delete [] ptr;
    }
  };

  inline static std::unordered_map<uint64_t,CStr> seen_map{};

  static uint64_t get_or_insert(const CStr& str, uint64_t hash){
    if(seen_map.contains(hash)){
      if(seen_map.at(hash)==str){
        return hash;
      }else{
        return get_or_insert(str,hash^rand());
      }
    }else{
      seen_map.emplace(hash,str);
      return hash;
    }
  }

  static uint64_t hash(const char* ptr,uint64_t len) {
    uint64_t hash=str_hash(ptr,len);
    CStr str(ptr,len);
    return get_or_insert(str,hash);
  }

  constexpr static uint64_t cstrlen(const char* ptr){
    uint64_t len=0;
    while(ptr[len]!=0){
      len++;
    }
    return len;
  }

public:
  uint64_t id=0;

  ID():id(0){}
  ID(const char* ptr):id(hash(ptr,cstrlen(ptr))){}
  ID(const char* ptr, uint64_t len):id(hash(ptr,len)){}
  ID(const std::string& str):id(hash(str.c_str(),str.length())){}

  operator const char*() const {
    if(id==0){
      return "";
    }
    return seen_map.at(id).ptr;
  }
  operator bool () const {
    return id;
  }

#define OPER(OP,TYPE)\
  bool operator OP (const TYPE& b) const{\
    return id OP ID(b).id;\
  }

#define CMP(TYPE) OPER(>,TYPE) OPER(<,TYPE) OPER(>=,TYPE) \
OPER(<=,TYPE) OPER(==,TYPE) OPER(!=,TYPE)

  CMP(ID)
  CMP(std::string)
  CMP(char*)
#undef OPER
#undef CMP
};

inline ID operator ""_id(const char* cstr,ulong len){
  return ID(cstr,len);
}

template<>
struct std::hash<ID>{
  uint64_t operator()(const ID& id) const{
    return id.id;
  }
};
