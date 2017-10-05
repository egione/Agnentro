/*
Bitscan
Copyright 2017 Russell Leidich

This collection of files constitutes the Bitscan Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Bitscan Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Bitscan Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Bitscan Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
These are better defined and portable than __builtin_clz(l) and related macros, which are specific to GCC and can actually be slower.

For least-significant-bit (LSB) search, use BITSCAN_LSB[8/16/32/64/_ULONG]_SMALL_GET if the value is expected to be have a log2 close to zero, BITSCAN_LSB[8/16/32/64/_ULONG]_BIG_GET if the log2 is expected to be close to the number of bits in the type, or BITSCAN_LSB[8/16/32/64/_ULONG]_FLAT_GET if there is no expectation. For most-significant-bit (MSB) search, use the same names with "MSB" instead. The LSB and MSB of zero are defined to be zero.
*/
#define BITSCAN_LSB16_BIG_GET(lsb, uint) \
  do{ \
    u32 _a; \
    \
    _a=(u8)(uint); \
    lsb=0; \
    if(!_a){ \
      _a=(uint)>>8; \
      if(uint){ \
        lsb=8; \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[_a]); \
  }while(0)

#define BITSCAN_LSB32_BIG_GET(lsb, uint) \
  do{ \
    u32 _a; \
    \
    _a=(uint)>>24; \
    lsb=0; \
    if((_a<<24)==(uint)){ \
      if(uint){ \
        lsb=24; \
      } \
    }else{ \
      _a=(uint)>>16; \
      if((_a<<16)==(uint)){ \
        lsb=16; \
      }else{ \
        _a=(uint)>>8; \
        if((_a<<8)==(uint)){ \
          lsb=8; \
        }else{ \
          _a=(uint); \
        } \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[(u8)(_a)]); \
  }while(0)

#define BITSCAN_LSB64_BIG_GET(lsb, uint) \
  do{ \
    u64 _a; \
    \
    _a=(uint)>>56; \
    lsb=0; \
    if((_a<<56)==(uint)){ \
      if(uint){ \
        lsb=56; \
      } \
    }else{ \
      _a=(uint)>>48; \
      if((_a<<48)==(uint)){ \
        lsb=48; \
      }else{ \
        _a=(uint)>>40; \
        if((_a<<40)==(uint)){ \
          lsb=40; \
        }else{ \
          _a=(uint)>>32; \
          if((_a<<32)==(uint)){ \
            lsb=32; \
          }else{ \
            _a=(uint)>>24; \
            if((_a<<24)==(uint)){ \
              lsb=24; \
            }else{ \
              _a=(uint)>>16; \
              if((_a<<16)==(uint)){ \
                lsb=16; \
              }else{ \
                _a=(uint)>>8; \
                if((_a<<8)==(uint)){ \
                  lsb=8; \
                }else{ \
                 _a=(uint); \
                } \
              } \
            } \
          } \
        } \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[(u8)(_a)]); \
  }while(0)

#define BITSCAN_LSB8_BIG_GET(lsb, uint) \
  lsb=bitscan_lsb_list_base[uint]

#define BITSCAN_LSB32_FLAT_GET(lsb, uint) \
  do{ \
    u32 _a; \
    u32 _b; \
    \
    _a=(uint)>>16; \
    lsb=0; \
    if((_a<<16)==(uint)){ \
      _b=(uint)>>24; \
      lsb=16; \
      if((_b<<24)==(uint)){ \
        lsb=0; \
        if(uint){ \
          _a=_b; \
          lsb=24; \
        } \
      } \
    }else{ \
      _b=(uint)>>8; \
      _a=(uint); \
      if((_b<<8)==(uint)){ \
        _a=_b; \
        lsb=8; \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[(u8)(_a)]); \
  }while(0)

#define BITSCAN_LSB64_FLAT_GET(lsb, uint) \
  do{ \
    u64 _a; \
    u64 _b; \
    \
    _a=(uint)>>32; \
    lsb=0; \
    if((_a<<32)==(uint)){ \
      _b=(uint)>>48; \
      if((_b<<48)==(uint)){ \
        _a=_b; \
        _b=(uint)>>56; \
        lsb=48; \
        if((_b<<56)==(uint)){ \
          lsb=0; \
          if(uint){ \
            _a=_b; \
            lsb=56; \
          } \
        } \
      }else{ \
        _b=(uint)>>40; \
        lsb=32; \
        if((_b<<40)==(uint)){ \
          _a=_b; \
          lsb=40; \
        } \
      } \
    }else{ \
      _b=(uint)>>16; \
      if((_b<<16)==(uint)){ \
        _a=_b; \
        _b=(uint)>>24; \
        lsb=16; \
        if((_b<<24)==(uint)){ \
          _a=_b; \
          lsb=24; \
        } \
      }else{ \
        _a=(uint); \
        _b=(uint)>>8; \
        if((_b<<8)==(uint)){ \
          _a=_b; \
          lsb=8; \
        } \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[(u8)(_a)]); \
  }while(0)

 #define BITSCAN_LSB32_SMALL_GET(lsb, uint) \
  do{ \
    u32 _a; \
    \
    _a=(u8)(uint); \
    lsb=0; \
    if(!_a){ \
      _a=(u8)((uint)>>8); \
      if(_a){ \
        lsb=8; \
      }else{ \
        _a=(u8)((uint)>>16); \
        lsb=16; \
        if(!_a){ \
          _a=(uint)>>24; \
          lsb=0; \
          if(uint){ \
            lsb=24; \
          } \
        } \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[_a]); \
  }while(0)

#define BITSCAN_LSB64_SMALL_GET(lsb, uint) \
  do{ \
    u64 _a; \
    \
    _a=(u8)(uint); \
    lsb=0; \
    if(!_a){ \
      _a=(u8)((uint)>>8); \
      if(_a){ \
        lsb=8; \
      }else{ \
        _a=(u8)((uint)>>16); \
        if(_a){ \
          lsb=16; \
        }else{ \
          _a=(u8)((uint)>>24); \
          if(_a){ \
            lsb=24; \
          }else{ \
            _a=(u8)((uint)>>32); \
            if(_a){ \
              lsb=32; \
            }else{ \
              _a=(u8)((uint)>>40); \
              if(_a){ \
                lsb=40; \
              }else{ \
                _a=(u8)((uint)>>48); \
                  lsb=48; \
                if(!_a){ \
                  _a=(uint)>>56; \
                  lsb=0; \
                  if(uint){ \
                    lsb=56; \
                  } \
                } \
              } \
            } \
          } \
        } \
      } \
    } \
    lsb=(u8)(lsb+bitscan_lsb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB16_BIG_GET(msb, uint) \
  do{ \
    u32 _a; \
    u32 _b; \
    \
    _a=(uint); \
    _b=(u32)(uint)>>8; \
    msb=0; \
    if(_b){ \
      _a=_b; \
      msb=8; \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB32_BIG_GET(msb, uint) \
  do{ \
    u32 _a; \
    \
    _a=(uint)>>24; \
    msb=0; \
    if(_a){ \
      msb=24; \
    }else{ \
      _a=(uint)>>16; \
      if(_a){ \
        msb=16; \
      }else{ \
        _a=(uint)>>8; \
        if(_a){ \
          msb=8; \
        }else{ \
          _a=(uint); \
        } \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB64_BIG_GET(msb, uint) \
  do{ \
    u64 _a; \
    \
    _a=(u32)((uint)>>56); \
    msb=0; \
    if(_a){ \
      msb=56; \
    }else{ \
      _a=(uint)>>48; \
      if(_a){ \
        msb=48; \
      }else{ \
        _a=(uint)>>40; \
        if(_a){ \
          msb=40; \
        }else{ \
          _a=(uint)>>32; \
          if(_a){ \
            msb=32; \
          }else{ \
            _a=(uint)>>24; \
            if(_a){ \
              msb=24; \
            }else{ \
              _a=(uint)>>16; \
              if(_a){ \
                msb=16; \
              }else{ \
                _a=(uint)>>8; \
                if(_a){ \
                  msb=8; \
                }else{ \
                  _a=(uint); \
                } \
              } \
            } \
          } \
        } \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB8_BIG_GET(msb, uint) \
  msb=bitscan_msb_list_base[uint]

#define BITSCAN_MSB32_FLAT_GET(msb, uint) \
  do{ \
    u32 _a; \
    u32 _b; \
    \
    _a=(uint)>>16; \
    msb=0; \
    if(_a){ \
      _b=(uint)>>24; \
      msb=16; \
      if(_b){ \
        _a=_b; \
        msb=24; \
      } \
    }else{ \
      _a=(uint); \
      _b=(uint)>>8; \
      if(_b){ \
        _a=_b; \
        msb=8; \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB64_FLAT_GET(msb, uint) \
  do{ \
    u64 _a; \
    u64 _b; \
    \
    _a=(uint)>>32; \
    msb=0; \
    if(_a){ \
      _b=(uint)>>48; \
      if(_b){ \
        _a=_b; \
        _b=(uint)>>56; \
        msb=48; \
        if(_b){ \
          _a=_b; \
          msb=56; \
        } \
      }else{ \
        _b=(uint)>>40; \
        msb=32; \
        if(_b){ \
          _a=_b; \
          msb=40; \
        } \
      } \
    }else{ \
      _b=(uint)>>16; \
      if(_b){ \
        _a=_b; \
        _b=(uint)>>24; \
        msb=16; \
        if(_b){ \
          _a=_b; \
          msb=24; \
        } \
      }else{ \
        _a=(uint); \
        _b=(uint)>>8; \
        if(_b){ \
          _a=_b; \
          msb=8; \
        } \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

 #define BITSCAN_MSB32_SMALL_GET(msb, uint) \
  do{ \
    u32 _a; \
    u32 _b; \
    \
    _a=(uint); \
    _b=(uint)>>8; \
    msb=0; \
    if(_b){ \
      _a=_b; \
      _b=(uint)>>16; \
      msb=8; \
      if(_b){ \
        _a=_b; \
        _b=(uint)>>24; \
        msb=16; \
        if(_b){ \
          _a=_b; \
          msb=24; \
        } \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#define BITSCAN_MSB64_SMALL_GET(msb, uint) \
  do{ \
    u64 _a; \
    u64 _b; \
    \
    _a=(uint); \
    _b=(uint)>>8; \
    msb=0; \
    if(_b){ \
      _a=_b; \
      _b=(uint)>>16; \
      msb=8; \
      if(_b){ \
        _a=_b; \
        _b=(uint)>>24; \
        msb=16; \
        if(_b){ \
          _a=_b; \
          _b=(uint)>>32; \
          msb=24; \
          if(_b){ \
            _a=_b; \
            _b=(uint)>>40; \
            msb=32; \
            if(_b){ \
              _a=_b; \
              _b=(uint)>>48; \
              msb=40; \
              if(_b){ \
                _a=_b; \
                _b=(uint)>>56; \
                msb=48; \
                if(_b){ \
                  _a=_b; \
                  msb=56; \
                } \
              } \
            } \
          } \
        } \
      } \
    } \
    msb=(u8)(msb+bitscan_msb_list_base[_a]); \
  }while(0)

#ifdef _64_
  #define BITSCAN_LSB_ULONG_BIG_GET BITSCAN_LSB64_BIG_GET
  #define BITSCAN_LSB_ULONG_FLAT_GET BITSCAN_LSB64_FLAT_GET
  #define BITSCAN_LSB_ULONG_SMALL_GET BITSCAN_LSB64_SMALL_GET
  #define BITSCAN_MSB_ULONG_BIG_GET BITSCAN_MSB64_BIG_GET
  #define BITSCAN_MSB_ULONG_FLAT_GET BITSCAN_MSB64_FLAT_GET
  #define BITSCAN_MSB_ULONG_SMALL_GET BITSCAN_MSB64_SMALL_GET
#elif defined(_32_)
  #define BITSCAN_LSB_ULONG_BIG_GET BITSCAN_LSB32_BIG_GET
  #define BITSCAN_LSB_ULONG_FLAT_GET BITSCAN_LSB32_FLAT_GET
  #define BITSCAN_LSB_ULONG_SMALL_GET BITSCAN_LSB32_SMALL_GET
  #define BITSCAN_MSB_ULONG_BIG_GET BITSCAN_MSB32_BIG_GET
  #define BITSCAN_MSB_ULONG_FLAT_GET BITSCAN_MSB32_FLAT_GET
  #define BITSCAN_MSB_ULONG_SMALL_GET BITSCAN_MSB32_SMALL_GET
#else
  #define BITSCAN_LSB_ULONG_BIG_GET BITSCAN_LSB16_BIG_GET
  #define BITSCAN_LSB_ULONG_FLAT_GET BITSCAN_LSB16_FLAT_GET
  #define BITSCAN_LSB_ULONG_SMALL_GET BITSCAN_LSB16_SMALL_GET
  #define BITSCAN_MSB_ULONG_BIG_GET BITSCAN_MSB16_BIG_GET
  #define BITSCAN_MSB_ULONG_FLAT_GET BITSCAN_MSB16_FLAT_GET
  #define BITSCAN_MSB_ULONG_SMALL_GET BITSCAN_MSB16_SMALL_GET
#endif

#define BITSCAN_LSB16_FLAT_GET BITSCAN_LSB16_BIG_GET
#define BITSCAN_LSB16_SMALL_GET BITSCAN_LSB16_BIG_GET
#define BITSCAN_LSB8_FLAT_GET BITSCAN_LSB8_BIG_GET
#define BITSCAN_LSB8_SMALL_GET BITSCAN_LSB8_BIG_GET
#define BITSCAN_MSB16_FLAT_GET BITSCAN_MSB16_BIG_GET
#define BITSCAN_MSB16_SMALL_GET BITSCAN_MSB16_BIG_GET
#define BITSCAN_MSB8_FLAT_GET BITSCAN_MSB8_BIG_GET
#define BITSCAN_MSB8_SMALL_GET BITSCAN_MSB8_BIG_GET
