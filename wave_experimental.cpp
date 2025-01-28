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


double calculate_frequency(char note, int octave, bool isSharp) {
    double baseFrequencies[] = {440, 494, 523, 587, 659, 698, 784};
    int noteIndex;
    switch (note) {
        case 'a':
        noteIndex = 0;
        break;
        case 'b': 
        noteIndex = 1; 
        break;
        case 'c': 
        noteIndex = 2; 
        break;
        case 'd': 
        noteIndex = 3; 
        break;
        case 'e': 
        noteIndex = 4; 
        break;
        case 'f': 
        noteIndex = 5; 
        break;
        case 'g': 
        noteIndex = 6; 
        break;

        default: noteIndex = 0; break;
    }

    double frequency = baseFrequencies[noteIndex];
    if (isSharp) {
        frequency *= pow(2, 1.0 / 12.0);
    }
    return frequency * pow(2, octave - 1);

}

int swap_endian_int(int integer) {

    int first_byte_mask = 0xff000000;
    int second_byte_mask = 0x00ff0000;
    int third_byte_mask = 0x0000ff00;
    int fourth_byte_mask = 0x000000ff;

    int first_int = (integer & first_byte_mask) >> 24;
    int second_int = (integer & second_byte_mask) >> 8;
    int third_int = (integer & third_byte_mask) << 8;
    int fourth_int = (integer & fourth_byte_mask) << 24;

    int final_int = first_int | second_int | third_int | fourth_int;

    return final_int;

}



void generate_sine_wave(short *buffer, double frequency, int numSamples, int sampleRate) {
    for (int i = 0; i < numSamples; i++) {
        double sample = cos(2.0 * M_PI * frequency * i / sampleRate);
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

    // Number of samples for 0.5 seconds
    int numSamples = SAMPLE_RATE * 0.5;
    short int buffer[numSamples];

    // Generate sine wave (440 Hz for 0.5 seconds)
    generate_sine_wave(buffer, 440, numSamples, SAMPLE_RATE);

    // Initialize WAV header
    wavheader_t wav_head;
    double sampleDuration = numSamples / (double)SAMPLE_RATE;
    set_header(wav_head, SAMPLE_RATE, 1, BITS_PER_SAMPLE, sampleDuration);

    // Open output file
    std::ofstream outfile("header_test.wav", std::ios::binary);

    // Write header and audio data
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wavheader_t));
    outfile.write(reinterpret_cast<char *>(buffer), numSamples * sizeof(short int));

    outfile.close();
    std::cout << "WAV file generated successfully." << std::endl;

    return 0;
}
