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

int main (int argc, char *argv[]) {
    std::ofstream outfile; // Initialize write file object.
    wavheader_t wav_head; // Initialize header.
    set_header(wav_head, 44100, 1, 16, 0.5); // Set header with default values.
    outfile.open("header_test.wav", std::ios::binary); // Open writefile in binary mode.
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wavheader_t)); 
    // cast address of header into char pointer in order to write to binary file.
    // Specify the size fo the data to write (in this case the whole header.)
    outfile.close(); // Close writefile.
    return 0;
}
