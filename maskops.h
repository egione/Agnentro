/*
MaskOps
Copyright 2017 Russell Leidich

This collection of files constitutes the MaskOps Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The MaskOps Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The MaskOps Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the MaskOps Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
#define SURROUND_U16(_dest, _max, _source, _surround) \
  if((_source)<=(_dest)){ \
    _surround=(_dest); \
    if(((_dest)>>1)<(_source)){ \
      _surround=(u16)(((_dest)-(_source))<<1); \
    } \
  }else{ \
    _surround=(u16)((_max)-(_dest)); \
    if((_surround>>1)<=((_max)-(_source))){ \
      _surround=(u16)((((_source)-(_dest))<<1)-1); \
    } \
  }

#define SURROUND_U24 SURROUND_U32
#define SURROUND_U32(_dest, _max, _source, _surround) \
  if((_source)<=(_dest)){ \
    _surround=(_dest); \
    if(((_dest)>>1)<(_source)){ \
      _surround=(u32)(((_dest)-(_source))<<1); \
    } \
  }else{ \
    _surround=(u32)((_max)-(_dest)); \
    if((_surround>>1)<=((_max)-(_source))){ \
      _surround=(u32)((((_source)-(_dest))<<1)-1); \
    } \
  }

#define SURROUND_U8(_dest, _max, _source, _surround) \
  if((_source)<=(_dest)){ \
    _surround=(_dest); \
    if(((_dest)>>1)<(_source)){ \
      _surround=(u8)(((_dest)-(_source))<<1); \
    } \
  }else{ \
    _surround=(u8)((_max)-(_dest)); \
    if((_surround>>1)<=((_max)-(_source))){ \
      _surround=(u8)((((_source)-(_dest))<<1)-1); \
    } \
  }

#define UNSURROUND_U16(_max, _source, _surround, _unsurround) \
  if((_source)<=((_max)>>1)){ \
    _unsurround=(_surround); \
    if(((_surround)>>1)<(_source)){ \
      _unsurround=(u16)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u16)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u16)((_source)-_unsurround-1); \
      } \
    } \
  }else{ \
    _unsurround=(u16)((_max)-(_surround)); \
    if(((_surround)>>1)<=((_max)-(_source))){ \
      _unsurround=(u16)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u16)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u16)((_source)-_unsurround-1); \
      } \
    } \
  }

#define UNSURROUND_U24 UNSURROUND_U32
#define UNSURROUND_U32(_max, _source, _surround, _unsurround) \
  if((_source)<=((_max)>>1)){ \
    _unsurround=(_surround); \
    if(((_surround)>>1)<(_source)){ \
      _unsurround=(u32)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u32)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u32)((_source)-_unsurround-1); \
      } \
    } \
  }else{ \
    _unsurround=(u32)((_max)-(_surround)); \
    if(((_surround)>>1)<=((_max)-(_source))){ \
      _unsurround=(u32)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u32)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u32)((_source)-_unsurround-1); \
      } \
    } \
  }

#define UNSURROUND_U8(_max, _source, _surround, _unsurround) \
  if((_source)<=((_max)>>1)){ \
    _unsurround=(_surround); \
    if(((_surround)>>1)<(_source)){ \
      _unsurround=(u8)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u8)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u8)((_source)-_unsurround-1); \
      } \
    } \
  }else{ \
    _unsurround=(u8)((_max)-(_surround)); \
    if(((_surround)>>1)<=((_max)-(_source))){ \
      _unsurround=(u8)((_surround)>>1); \
      if(!((_surround)&1)){ \
        _unsurround=(u8)((_source)+_unsurround); \
      }else{ \
        _unsurround=(u8)((_source)-_unsurround-1); \
      } \
    } \
  }
