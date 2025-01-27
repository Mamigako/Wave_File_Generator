#include <fstream>

struct wavheader_t {
    unsigned char RIFF[4];
    unsigned int chunkSize;
    unsigned char WAVE[4];
    unsigned char fmt[4];
    unsigned int subChunk1Size;
    unsigned short int audioFormat;
    unsigned short int numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short int blockAlign;
    unsigned short int bitsPerSample;
    unsigned char DATA[4];
    unsigned int subChunk2Size;
    wavheader_t(){
        unsigned char temp_riff[4] = {'R','I','F','F'};
        unsigned char temp_wave[4] = {'W','A','V','E'};
        unsigned char temp_fmt[4] = {'f','m','t',' '};
        unsigned char temp_data[4] = {'d','a','t','a'};
        for(int i = 0; i < 4; i++){
            RIFF[i] = temp_riff[i];
            WAVE[i] = temp_wave[i];
            fmt[i] = temp_fmt[i];
            DATA[i] = temp_data[i];
        }
    }
};

void set_header(wavheader_t &wav_head, int sampleRate, int noChannels, int bitsSample, double sampleDuration){
    int pmc_size = 16;
    short int pmc = 1;
    int numSamples = sampleDuration * sampleRate;
    wav_head.subChunk1Size = pmc_size;
    wav_head.audioFormat = pmc;
    wav_head.numChannels = noChannels;
    wav_head.sampleRate = sampleRate;
    wav_head.byteRate = sampleRate * noChannels * bitsSample/8;
    wav_head.blockAlign = noChannels * bitsSample/8;
    wav_head.bitsPerSample = bitsSample;
    wav_head.subChunk2Size = numSamples * noChannels * bitsSample/8;
    wav_head.chunkSize = 4 + (8 + wav_head.subChunk1Size) + (8 + wav_head.subChunk2Size);
}

int main (int argc, char *argv[]) {
    std::ofstream outfile;
    wavheader_t wav_head;
    set_header(wav_head, 44100, 1, 16, 0.5);
    outfile.open("header_test.wav", std::ios::binary);
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wavheader_t));
    outfile.close();
    return 0;
}
