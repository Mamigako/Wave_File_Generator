#include <cstring>
#include <fstream>
#include <cmath>
#include <iostream>

struct wavheader_t
{   // RIFF header
    unsigned char RIFF[4];
    unsigned int chunkSize;
    unsigned char WAVE[4];
    // fmt chunk
    unsigned char fmt[4];
    unsigned int subChunk1Size;
    unsigned short int audioFormat;
    unsigned short int numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short int blockAlign;
    unsigned short int bitsPerSample;
    // data chunk
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

/*
    Calculates the frequency based on the note and its octave.
*/
double calculate_frequency(char note, int octave)
{
    bool isSharp = false;
    double baseFrequencies[] = {440, 494, 523, 587, 659, 698, 784};
    int noteIndex;

    if (64 < note && note < 72) // We use ASCII values to determine whether the note is sharp or not 
    {
        isSharp = true;
        note += 32;
    }

    switch (note)
    {
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

    default:
        noteIndex = 0;
        break;
    }

    double frequency = baseFrequencies[noteIndex];
    if (isSharp)
    {
        frequency *= pow(2.0, 1.0 / 12.0); // Calculate the frequency if sharp.
    }
    return frequency * pow(2, octave - 1); // Calculate the correct frequency using the octave
}

/*
    Takes in the note subdivison and calculates its duration based on quarter note duration. 
*/
float calculate_duration(int numerator, int denominator, float quarterNoteDuration)
{
    float barDuration = quarterNoteDuration * 4.0;
    float duration = barDuration * ((float)numerator / (float)denominator);

    return duration;
}

/*
    Sets the header.
*/
void set_header(wavheader_t &wav_head, int sampleRate, int noChannels, int bitsSample, double sampleDuration)
{
    int pmc_size = 16;
    short int pmc = 1;
    int numSamples = sampleDuration * sampleRate;                      // varies based on the duration
    wav_head.subChunk1Size = pmc_size;                                 // 16 bytes
    wav_head.audioFormat = pmc;                                        // 1 (Pulse Code Modulation)
    wav_head.numChannels = noChannels;                                 // 1 for Mono
    wav_head.sampleRate = sampleRate;                                  // eg. 44100
    wav_head.byteRate = sampleRate * noChannels * bitsSample / 8;      // Bytes per second
    wav_head.blockAlign = noChannels * bitsSample / 8;                 // Calculate the number of bytes for one audio frame
    wav_head.bitsPerSample = bitsSample;                               // 16
    wav_head.subChunk2Size = numSamples * noChannels * bitsSample / 8; // Data Subchunk, calculates the size of audio data in bytes
    wav_head.chunkSize = 4 + (8 + wav_head.subChunk1Size) + (8 + wav_head.subChunk2Size);
    // Total size of RIFF chunk(4bytes for "WAVE", 8 bytes for "fmt" subchnunk header,
    // 8 bytes for "data" subchunk header, actual sizes of "fmt" and "data")
}

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        std::cerr << "Error: A file must be provided" << std::endl;
        return -1;
    }

    const int SAMPLE_RATE = 44100;
    const int BITS_PER_SAMPLE = 16;

    char file_name[37];
    int bpm;
    float duration = 0;
    float quarterNoteDuration;
    double frequency;

    int numSamples;
    char readBuffer[36];

    std::ifstream inputFile; // Initialize read file object.
    std::ofstream outfile;   // Initialize write file object.

    inputFile.open(argv[1]);
    inputFile.seekg(0);
    int line = 1;

    wavheader_t wav_head;
    set_header(wav_head, SAMPLE_RATE, 1, BITS_PER_SAMPLE, duration);

    while (inputFile.getline(readBuffer, sizeof(readBuffer)))
    {
        if (line == 1)
        {
            std::strncpy(file_name, readBuffer, strlen(readBuffer) + 1);
            std::strcat(file_name, ".wav"); // concatenate the file name with .wav
            outfile.open(file_name, std::ios::binary | std::ios::app); // Open writefile in binary mode.
        }
        else if (line == 2)
        {
            bpm = atoi(readBuffer);
            quarterNoteDuration = 60.0 / bpm;
            numSamples = SAMPLE_RATE * quarterNoteDuration;
        }
        else
        {
            // Handle the silence
            if (readBuffer[0] == 's')
            {
                int numerator = ((int)readBuffer[2]) - 48; // cast to int to parse the int and then subtract the ASCII value 48
                int denominator = atoi(&readBuffer[4]);    // we had to use atoi() to grab the 1 and 2 in 12
                float silenceDuration = calculate_duration(numerator, denominator, quarterNoteDuration);

                duration += silenceDuration;
                numSamples = SAMPLE_RATE * silenceDuration;
                int silence = 0;

                for (int i = 0; i < numSamples; i++)
                {
                    outfile.write(reinterpret_cast<char *>(&silence), sizeof(short int));
                }
            }
            // Handle the notes
            else
            {
                char note = readBuffer[0];
                int octave = ((int)readBuffer[2]) - 48; 
                int numerator = ((int)readBuffer[4]) - 48;
                int denominator = atoi(&readBuffer[6]);

                float noteDuration = calculate_duration(numerator, denominator, quarterNoteDuration);
                duration += noteDuration;
                numSamples = SAMPLE_RATE * noteDuration;

                frequency = calculate_frequency(note, octave);

                for (int i = 0; i < numSamples; i++)
                {
                    double sample = cos(3.142 * frequency * i / SAMPLE_RATE);
                    short int sampleBytes = (short int)(sample * 32767);

                    outfile.write(reinterpret_cast<char *>(&sampleBytes), sizeof(short int));
                }
            }
        }

        line += 1;
    }

    outfile.close(); // we had to close it to be able to open it again in a different mode to re-write the header after finding the total duration
    outfile.open(file_name, std::ios::binary | std::ios::in);

    outfile.seekp(0);
    set_header(wav_head, SAMPLE_RATE, 1, BITS_PER_SAMPLE, duration);
    outfile.write(reinterpret_cast<char *>(&wav_head), sizeof(wavheader_t));

    inputFile.close(); // Close read file
    outfile.close(); // Close writefile
    std::cout << "WAV file generated successfully." << std::endl;

    return 0;
}
