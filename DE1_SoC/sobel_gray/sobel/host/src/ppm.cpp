#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>

#include "ppm.h"

bool 
parse_ppm_header(const char *filename, unsigned int *width, unsigned int *height, unsigned int *channels) {
  FILE *fp = NULL;
#ifdef _WIN32
  errno_t err;
  if ((err = fopen_s(&fp, filename, "rb")) != 0)
#else
  if ((fp = fopen(filename, "rb")) == 0)
#endif
  {
    if (fp) { fclose(fp); }
    std::cerr << "Error: failed to load '" << filename << "'" << std::endl;
    return false;
  }

  const size_t headerSize = 0x40;
  char header[headerSize];
  if ((fgets(header, headerSize, fp) == NULL) && ferror(fp)) {
    if (fp) { fclose(fp); }
    std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
    return false;
  }

  if (strncmp(header, "P5", 2) == 0) {
    *channels = 1;
  } else if (strncmp(header, "P6", 2) == 0) {
    *channels = 3;
  } else {
    std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
    return false;
  }

  int i = 0;
  unsigned int maxval = 0;
  while (i < 3) {
    if ((fgets(header, headerSize, fp) == NULL) && ferror(fp)) {
      if (fp) { fclose(fp); }
      std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
      return false;
    }
    // Skip comments
    if (header[0] == '#') continue;
#ifdef _WIN32
    if (i == 0) {
      i += sscanf_s(header, "%u %u %u", width, height, &maxval);
    } else if (i == 1) {
      i += sscanf_s(header, "%u %u", height, &maxval);
    } else if (i == 2) {
      i += sscanf_s(header, "%u", &maxval);
    }
#else
    if (i == 0) {
      i += sscanf(header, "%u %u %u", width, height, &maxval);
    } else if (i == 1) {
      i += sscanf(header, "%u %u", height, &maxval);
    } else if (i == 2) {
      i += sscanf(header, "%u", &maxval);
    }
#endif
  }

  if (fp) { fclose(fp); }
  
  return true;
}

bool 
parse_ppm_data(const char *filename, unsigned int *width, unsigned int *height, unsigned int *channels, unsigned char *data) {
  FILE *fp = NULL;
#ifdef _WIN32
  errno_t err;
  if ((err = fopen_s(&fp, filename, "rb")) != 0)
#else
  if ((fp = fopen(filename, "rb")) == 0)
#endif
  {
    if (fp) { fclose(fp); }
    std::cerr << "Error: failed to load '" << filename << "'" << std::endl;
    return false;
  }

  const size_t headerSize = 0x40;
  char header[headerSize];
  if ((fgets(header, headerSize, fp) == NULL) && ferror(fp)) {
    if (fp) { fclose(fp); }
    std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
    return false;
  }

  if (strncmp(header, "P5", 2) == 0) {
    *channels = 1;
  } else if (strncmp(header, "P6", 2) == 0) {
    *channels = 3;
  } else {
    std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
    return false;
  }
  
  int i = 0;
  unsigned int maxval = 0;
  while (i < 3) {
    if ((fgets(header, headerSize, fp) == NULL) && ferror(fp)) {
      if (fp) { fclose(fp); }
      std::cerr << "Error: '" << filename << "' is not a valid PPM image" << std::endl;
      return false;
    }
    // Skip comments
    if (header[0] == '#') continue;
#ifdef _WIN32
    if (i == 0) {
      i += sscanf_s(header, "%u %u %u", width, height, &maxval);
    } else if (i == 1) {
      i += sscanf_s(header, "%u %u", height, &maxval);
    } else if (i == 2) {
      i += sscanf_s(header, "%u", &maxval);
    }
#else
    if (i == 0) {
      i += sscanf(header, "%u %u %u", width, height, &maxval);
    } else if (i == 1) {
      i += sscanf(header, "%u %u", height, &maxval);
    } else if (i == 2) {
      i += sscanf(header, "%u", &maxval);
    }
#endif
  }

  if (maxval == 0) {
    if (fp) { fclose(fp); }
    std::cerr << "Error: maximum color value must be greater than 0" << std::endl;
    return false;
  }
  if (maxval > 255) {
    if (fp) { fclose(fp); }
    std::cerr << "Error: parser only supports 1 byte value PPM images" << std::endl;
    return false;
  }
  
  unsigned char *raw = (unsigned char *)malloc(sizeof(unsigned char) * *width * *height * *channels);
  if (!raw) {
    if (fp) { fclose(fp); }
    std::cerr << "Error: could not allocate data buffer" << std::endl;
    return false;
  }
  
  if (fread(raw, sizeof(unsigned char), *width * *height * *channels, fp) != *width * *height * *channels) {
    if (fp) fclose(fp);
    std::cerr << "Error: invalid image data" << std::endl;
    return false;
  }
  
  if (fp) fclose(fp);

  // Transfer the raw data
  unsigned char *raw_ptr = raw;
  
  if (*channels == 1) {
    unsigned int *data_ptr = (unsigned int *) data;
    for (int i = 0, e = *width * *height; i != e; ++i) {
      *data_ptr++ = *raw_ptr++;
    }
  } else {
    unsigned char *data_ptr = data;
    for (int i = 0, e = *width * *height; i != e; ++i) {
      // Read channels and pad as necessary (Little Endian)   
      *data_ptr++ = *raw_ptr++; // B
      *data_ptr++ = *raw_ptr++; // G
      *data_ptr++ = *raw_ptr++; // R
      *data_ptr++ = 0;
    }
  }
  free(raw);  
  
  return true;
}

// Dump frame data in PPM format.
void 
dump_ppm_data(std::string filename, unsigned int width, unsigned int height, unsigned int channels, unsigned int *data) {
  std::cout << "Dumping " << filename << std::endl;
  FILE *f = fopen(filename.c_str(), "wb");
  if (f != NULL) {
    if (channels == 1) {
      fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);
      for(unsigned int y = 0; y < height; ++y) {
        for(unsigned int x = 0; x < width; ++x) {
          // This assumes byte-order is little-endian.
          unsigned char pixel = (unsigned char) data[y * width + x];
          fwrite(&pixel, 1, 1, f);
        }
      }
      fwrite(data, 1, width*height, f);
    } else if (channels == 3) {
      fprintf(f, "P6\n%d %d\n%d\n", width, height, 255);
      for(unsigned int y = 0; y < height; ++y) {
        for(unsigned int x = 0; x < width; ++x) {
          // This assumes byte-order is little-endian.
          unsigned int pixel = data[y * width + x];
          fwrite(&pixel, 1, 3, f);
        }
      }
    } else {
      std::cout << "Cannot write this ppm file (wrong channel count: " << channels << ")" << std::endl;
    }
    fclose(f);
  }
}