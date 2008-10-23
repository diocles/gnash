//   Copyright (C) 2008 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "AudioDecoder.h"

#include <speex/speex.h> 

#ifndef GNASH_MEDIA_DECODER_SPEEX
#define GNASH_MEDIA_DECODER_SPEEX

namespace gnash {
namespace media {

class AudioDecoderSpeex : public AudioDecoder
{
public:
    AudioDecoderSpeex();
    ~AudioDecoderSpeex();

    boost::uint8_t* decode(const EncodedAudioFrame& input,
        boost::uint32_t& outputSize);

private:

    SpeexBits _speex_bits;
    void* _speex_dec_state;
    int _speex_framesize;
};

} // namespace media
} // namespace gnash

#endif // GNASH_MEDIA_DECODER_SPEEX