#include <fstream>
#include <cmath>
#include <iostream>

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

void generate_sine_wave(short *buffer, double frequency, int numSamples, int sampleRate) {
    for (int i = 0; i < numSamples; i++) {
        double sample = cos(3.142 * frequency * i / sampleRate);
        buffer[i] = (short int)(sample * 32767); // Scale to 16-bit range
    }
}

void set_header(wavheader_t &wav_head, int sampleRate, int noChannels, int bitsSample, double sampleDuration){
    int pmc_size = 16;
    short int pmc = 1;
    int numSamples = sampleDuration * sampleRate; // 0.5 * 44100
    wav_head.subChunk1Size = pmc_size; // 16 bytes
    wav_head.audioFormat = pmc; // 1 (Pulse Code Modulation) 
    wav_head.numChannels = noChannels; // 1 for Mono
    wav_head.sampleRate = sampleRate; // eg. 44100
    wav_head.byteRate = sampleRate * noChannels * bitsSample/8; // Bytes per second
    wav_head.blockAlign = noChannels * bitsSample/8; // Calculate the number of bytes for one audio frame
    wav_head.bitsPerSample = bitsSample; // 16
    wav_head.subChunk2Size = numSamples * noChannels * bitsSample/8; // Data Subchunk, calculates the size of audio data in bytes
    wav_head.chunkSize = 4 + (8 + wav_head.subChunk1Size) + (8 + wav_head.subChunk2Size); 
    //Total size of RIFF chunk(4bytes for "WAVE", 8 bytes for "fmt" subchnunk header, 
    //8 bytes for "data" subchunk header, actual sizes of "fmt" and "data")

}

int main(int argc, char *argv[]) {
    const int SAMPLE_RATE = 44100;
    const int BITS_PER_SAMPLE = 16;
    double sampleDuration = 0.5;

    // Number of samples for 0.5 seconds
    int numSamples = SAMPLE_RATE * sampleDuration;
    short int buffer[numSamples];

    // Initialize WAV header
    wavheader_t wav_head;
    set_header(wav_head, SAMPLE_RATE, 1, BITS_PER_SAMPLE, sampleDuration);

    generate_sine_wave(buffer, 440, numSamples, SAMPLE_RATE);

    std::ofstream outfile("header_test.wav", std::ios::binary);
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wavheader_t));
    outfile.write(reinterpret_cast<char *>(buffer), numSamples * sizeof(short int));

    outfile.close();
    std::cout << "WAV file generated successfully." << std::endl;

    return 0;
}
