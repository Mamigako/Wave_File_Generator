#include <fstream>
#include <cmath>

const int maxNotes = 500;

struct wavheader_t
{
    // RIFF header
    unsigned char RIFF[4];
    unsigned int chunkSize;
    unsigned char WAVE[4];
    // fmt  subchunk
    unsigned char fmt[4];
    unsigned int subChunk1Size;
    unsigned short int audioFormat;
    unsigned short int numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short int blockAlign;
    unsigned short int bitsPerSample;
    // data subchunk
    unsigned char DATA[4];
    unsigned int subChunk2Size;
    wavheader_t()
    {
        unsigned char temp_riff[4] = {'R', 'I', 'F', 'F'};
        unsigned char temp_wave[4] = {'W', 'A', 'V', 'E'};
        unsigned char temp_fmt[4] = {'f', 'm', 't', ' '};
        unsigned char temp_data[4] = {'d', 'a', 't', 'a'};
        for (int i = 0; i < 4; i++)
        {
            RIFF[i] = temp_riff[i];
            WAVE[i] = temp_wave[i];
            fmt[i] = temp_fmt[i];
            DATA[i] = temp_data[i];
        }
    }
};

double calculate_frequency(char note, int octave, bool isSharp)
{
    double baseFrequencuies[] = {440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880};
    int noteIndex;
    switch (note)
    {
    case 'a':
        noteIndex = 7;
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

    default:
        noteIndex = 0;
        break;
    }

    double frequency = baseFrequencuies[noteIndex];
    if (isSharp)
    {
        frequency *= pow(2, 1 / 12);
    }
    return frequency * pow(2, octave - 1);
}

void set_header(wavheader_t &wav_head, int sampleRate, int noChannels, int bitsSample, double sampleDuration)
{
    int pmc_size = 16;
    short int pmc = 1;
    int numSamples = sampleDuration * sampleRate;                        // 0.5 * 44100
    wav_head.subChunk1Size = pmc_size;                                   // 16 bytes
    wav_head.audioFormat = pmc;                                          // 1 (Pulse Code Modulation)
    wav_head.numChannels = noChannels;                                   // 1 for Mono
    wav_head.sampleRate = sampleRate;                                    // eg. 44100
    wav_head.byteRate = sampleRate * noChannels * (bitsSample / 8);      // Bytes per second
    wav_head.blockAlign = noChannels * (bitsSample / 8);                 // Calculate the number of bytes for one audio frame
    wav_head.bitsPerSample = bitsSample;                                 // 16
    wav_head.subChunk2Size = numSamples * noChannels * (bitsSample / 8); // Data Subchunk, calculates the size of audio data in bytes
    wav_head.chunkSize = 4 + (8 + wav_head.subChunk1Size) + (8 + wav_head.subChunk2Size);
    // Total size of RIFF chunk(4bytes for "WAVE", 8 bytes for "fmt" subchnunk header,
    // 8 bytes for "data" subchunk header, actual sizes of "fmt" and "data")
}

int main(int argc, char *argv[])
{
    int sampleRate = 44100;
    int freq = 440;
    double duration = 0.5;
    short numChannels = 1;
    short bitsPerSample = 16;

    int numSamples = duration * sampleRate;

    wavheader_t wav_head;                                                   // Initialize header.
    set_header(wav_head, sampleRate, numChannels, bitsPerSample, duration); // Set header with default values.
    std::ofstream outfile("test.wav", std::ios::binary);                    // Initialize write file object.
    // outfile.open("header_test.wav", std::ios::binary); // Open writefile in binary mode.
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wav_head));
    // cast address of header into char pointer in order to write to binary file.
    // Specify the size fo the data to write (in this case the whole header.)

    for (int i = 0; i < numSamples; i++)
    {
        double wave = std::cos(freq * i * 3.142 / sampleRate);
        short sampleInt = static_cast<short>(wave * 32767);

        outfile.write(reinterpret_cast<const char *>(&sampleInt), 2);
    }
    outfile.close(); // Close writefile.
    return 0;
}
