#include "texture.h"
#include "CGL/color.h"

#include <cmath>
#include <algorithm>

namespace CGL {
  Color lerp(float x, Color c1, Color c2){
    Color c = c1 + x * (c2 - c1);
    c.g = min(1.0f, max(0.0f, c.g));
    c.r = min(1.0f, max(0.0f, c.r));
    c.b = min(1.0f, max(0.0f, c.b));
    return c;
  }

  Color Texture::sample(const SampleParams& sp) {
    // TODO: Task 6: Fill this in.
    float level = 0;
    
    if(sp.lsm == L_ZERO){
    //   cout<<"mode1"<<endl;
      if(sp.psm == P_NEAREST) return sample_nearest(sp.p_uv, 0);
      else if(sp.psm == P_LINEAR) return sample_bilinear(sp.p_uv, 0);
    }
    else if(sp.lsm == L_NEAREST){
      level = get_level(sp);
      return sample_bilinear(sp.p_uv, (int)level);
    }
    else {
    //   cout<<"mode3"<<endl;

      level = get_level(sp);
      int up = ceil(level), low = floor(level);
      Color c1 = sample_bilinear(sp.p_uv, low);
      Color c2 = sample_bilinear(sp.p_uv, up);
      return lerp(level - low, c1, c2);
    }

// return magenta for invalid level
    return Color(1, 0, 1);
  }

  float Texture::get_level(const SampleParams& sp) {
    // TODO: Task 6: Fill this in.
    Vector2D dx = sp.p_dx_uv - sp.p_uv;
    Vector2D dy = sp.p_dy_uv - sp.p_uv;
    float L1 = pow(dx.x * width, 2) + pow(dx.y * height, 2);
    float L2 = pow(dy.x * width, 2) + pow(dy.y * height, 2);

    float L = max(sqrt(L1), sqrt(L2));
    return max(0.0f, log2(L));

  }

  Color MipLevel::get_texel(size_t tx, size_t ty) {
    if (texels.size() < tx * 3 + ty * width * 3 + 1) {
        cout<<tx<<" "<<ty<<" "<<tx*3+ty*width*3<<" "<<width<<" "<<height <<" "<<texels.size()<<endl;
        cout << "Error in get_texels\n";
        exit(0);
    }
    return Color(&texels[tx * 3 + ty * width * 3]);
  }

  Color Texture::sample_nearest(Vector2D uv, int level) {
    // TODO: Task 5: Fill this in.

    auto& mip = mipmap[level];
    float realx = mip.width * uv.x, realy = mip.height * uv.y;
    size_t x = abs(floor(realx) - realx) < abs(ceil(realx) - realx) ? floor(realx) : ceil(realx);
    size_t y = abs(floor(realy) - realy) < abs(ceil(realy) - realy) ? floor(realy) : ceil(realy);
    x = min(mip.width - 1, max((size_t)0, x));
    y = min(mip.height - 1, max((size_t)0, y));

    return mip.get_texel(x, y);



    // return magenta for invalid level
  }

  Color Texture::sample_bilinear(Vector2D uv, int level) {
    // TODO: Task 5: Fill this in.
    auto& mip = mipmap[level];
    float realx = mip.width * uv.x, realy = mip.height * uv.y;
    size_t floorx = floor(realx), ceilx = ceil(realx), floory = floor(realy), ceily = ceil(realy);
    floorx = min(mip.width - 1, max((size_t)0, floorx));
    floory = min(mip.height - 1, max((size_t)0, floory));
    ceilx = min(mip.width - 1, max((size_t)0, ceilx));
    ceily = min(mip.height - 1, max((size_t)0, ceily));
    // cout<<floorx<<" "<<floory<<" "<<ceilx<<" "<<ceily<<" "<<width<<" "<<height<<" "<<endl;
    Color c00 = mip.get_texel(floorx, floory), c10 = mip.get_texel(ceilx, floory);
    Color c01 = mip.get_texel(floorx, ceily), c11 = mip.get_texel(ceilx, ceily);
    float t = realy - floory, s = realx - floorx;
    Color c0 = lerp(s, c00, c10), c1 = lerp(s, c01, c11);
    return lerp(t, c0, c1);



    // return magenta for invalid level
    
  }



  /****************************************************************************/

  // Helpers

  inline void uint8_to_float(float dst[3], unsigned char* src) {
    uint8_t* src_uint8 = (uint8_t*)src;
    dst[0] = src_uint8[0] / 255.f;
    dst[1] = src_uint8[1] / 255.f;
    dst[2] = src_uint8[2] / 255.f;
  }

  inline void float_to_uint8(unsigned char* dst, float src[3]) {
    uint8_t* dst_uint8 = (uint8_t*)dst;
    dst_uint8[0] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[0])));
    dst_uint8[1] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[1])));
    dst_uint8[2] = (uint8_t)(255.f * max(0.0f, min(1.0f, src[2])));
  }

  void Texture::generate_mips(int startLevel) {

    // make sure there's a valid texture
    if (startLevel >= mipmap.size()) {
      std::cerr << "Invalid start level";
    }

    // allocate sublevels
    int baseWidth = mipmap[startLevel].width;
    int baseHeight = mipmap[startLevel].height;
    int numSubLevels = (int)(log2f((float)max(baseWidth, baseHeight)));

    numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
    mipmap.resize(startLevel + numSubLevels + 1);

    int width = baseWidth;
    int height = baseHeight;
    for (int i = 1; i <= numSubLevels; i++) {

      MipLevel& level = mipmap[startLevel + i];

      // handle odd size texture by rounding down
      width = max(1, width / 2);
      //assert (width > 0);
      height = max(1, height / 2);
      //assert (height > 0);

      level.width = width;
      level.height = height;
      level.texels = vector<unsigned char>(3 * width * height);
    }

    // create mips
    int subLevels = numSubLevels - (startLevel + 1);
    for (int mipLevel = startLevel + 1; mipLevel < startLevel + subLevels + 1;
      mipLevel++) {

      MipLevel& prevLevel = mipmap[mipLevel - 1];
      MipLevel& currLevel = mipmap[mipLevel];

      int prevLevelPitch = prevLevel.width * 3; // 32 bit RGB
      int currLevelPitch = currLevel.width * 3; // 32 bit RGB

      unsigned char* prevLevelMem;
      unsigned char* currLevelMem;

      currLevelMem = (unsigned char*)&currLevel.texels[0];
      prevLevelMem = (unsigned char*)&prevLevel.texels[0];

      float wDecimal, wNorm, wWeight[3];
      int wSupport;
      float hDecimal, hNorm, hWeight[3];
      int hSupport;

      float result[3];
      float input[3];

      // conditional differentiates no rounding case from round down case
      if (prevLevel.width & 1) {
        wSupport = 3;
        wDecimal = 1.0f / (float)currLevel.width;
      }
      else {
        wSupport = 2;
        wDecimal = 0.0f;
      }

      // conditional differentiates no rounding case from round down case
      if (prevLevel.height & 1) {
        hSupport = 3;
        hDecimal = 1.0f / (float)currLevel.height;
      }
      else {
        hSupport = 2;
        hDecimal = 0.0f;
      }

      wNorm = 1.0f / (2.0f + wDecimal);
      hNorm = 1.0f / (2.0f + hDecimal);

      // case 1: reduction only in horizontal size (vertical size is 1)
      if (currLevel.height == prevLevel.height) {
        //assert (currLevel.height == 1);

        for (int i = 0; i < currLevel.width; i++) {
          wWeight[0] = wNorm * (1.0f - wDecimal * i);
          wWeight[1] = wNorm * 1.0f;
          wWeight[2] = wNorm * wDecimal * (i + 1);

          result[0] = result[1] = result[2] = 0.0f;

          for (int ii = 0; ii < wSupport; ii++) {
            uint8_to_float(input, prevLevelMem + 3 * (2 * i + ii));
            result[0] += wWeight[ii] * input[0];
            result[1] += wWeight[ii] * input[1];
            result[2] += wWeight[ii] * input[2];
          }

          // convert back to format of the texture
          float_to_uint8(currLevelMem + (3 * i), result);
        }

        // case 2: reduction only in vertical size (horizontal size is 1)
      }
      else if (currLevel.width == prevLevel.width) {
        //assert (currLevel.width == 1);

        for (int j = 0; j < currLevel.height; j++) {
          hWeight[0] = hNorm * (1.0f - hDecimal * j);
          hWeight[1] = hNorm;
          hWeight[2] = hNorm * hDecimal * (j + 1);

          result[0] = result[1] = result[2] = 0.0f;
          for (int jj = 0; jj < hSupport; jj++) {
            uint8_to_float(input, prevLevelMem + prevLevelPitch * (2 * j + jj));
            result[0] += hWeight[jj] * input[0];
            result[1] += hWeight[jj] * input[1];
            result[2] += hWeight[jj] * input[2];
          }

          // convert back to format of the texture
          float_to_uint8(currLevelMem + (currLevelPitch * j), result);
        }

        // case 3: reduction in both horizontal and vertical size
      }
      else {

        for (int j = 0; j < currLevel.height; j++) {
          hWeight[0] = hNorm * (1.0f - hDecimal * j);
          hWeight[1] = hNorm;
          hWeight[2] = hNorm * hDecimal * (j + 1);

          for (int i = 0; i < currLevel.width; i++) {
            wWeight[0] = wNorm * (1.0f - wDecimal * i);
            wWeight[1] = wNorm * 1.0f;
            wWeight[2] = wNorm * wDecimal * (i + 1);

            result[0] = result[1] = result[2] = 0.0f;

            // convolve source image with a trapezoidal filter.
            // in the case of no rounding this is just a box filter of width 2.
            // in the general case, the support region is 3x3.
            for (int jj = 0; jj < hSupport; jj++)
              for (int ii = 0; ii < wSupport; ii++) {
                float weight = hWeight[jj] * wWeight[ii];
                uint8_to_float(input, prevLevelMem +
                  prevLevelPitch * (2 * j + jj) +
                  3 * (2 * i + ii));
                result[0] += weight * input[0];
                result[1] += weight * input[1];
                result[2] += weight * input[2];
              }

            // convert back to format of the texture
            float_to_uint8(currLevelMem + currLevelPitch * j + 3 * i, result);
          }
        }
      }
    }
  }

}
