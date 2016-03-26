bool parse_ppm_header(const char *filename, unsigned int *width, unsigned int *height, unsigned int *channels);
bool parse_ppm_data(const char *filename, unsigned int *width, unsigned int *height, unsigned int *channels, unsigned char *data);
void dump_ppm_data(std::string filename, unsigned int width, unsigned int height, unsigned int channels, unsigned int *data);