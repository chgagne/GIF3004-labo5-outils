// Exemple pour décoder un fichier raw opus encodé avec encode_opus.c
// Mathieu Garon (mathieu.garon.2@ulaval.ca) 2019

#include <stdio.h> /* Standard input/output definitions */
#include <string.h> /* String function definitions */
#include <unistd.h> /* UNIX standard function definitions */
#include <fcntl.h> /* File control definitions */
#include <errno.h> /* Error number definitions */
#include <stdlib.h> 
#include <string.h>
#include <signal.h>
#include <opus/opus.h>
#include <sys/mman.h>

typedef unsigned char uchar;

// Information sur le header WAV:
//https://web.archive.org/web/20140327141505/https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
//http://www.topherlee.com/software/pcm-tut-wavformat.html

typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int32_t ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */

    char    Subchunk1ID[4]; /*  4   */
    int32_t Subchunk1Size;  /*  4   */
    int16_t AudioFormat;    /*  2   */
    int16_t NumChannels;    /*  2   */
    int32_t SampleRate;     /*  4   */
    int32_t ByteRate;       /*  4   */
    int16_t BlockAlign;     /*  2   */
    int16_t BitsPerSample;  /*  2   */

    char    Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;

// Fonction pour placer le fichier en mémoire
uchar* map_file(const char* fn, size_t* size) {
    FILE* fp = fopen(fn, "r");
    if(!fp) {
        printf("fail to open file: %s", fn);
        return (uchar*)MAP_FAILED;
    }

    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    uchar* ptr = (uchar*)mmap(NULL, pos, PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);
    *size = pos;
    fclose(fp);
    return ptr;
}

int main(int argc, char *argv[])
{
    int ret;
    size_t file_size = 0;

    uchar* input_ptr = map_file(argv[1], &file_size); // Pointeur de début de fichier
    if (input_ptr == MAP_FAILED) {
        printf("fail to map file");
        return -1;
    }
    uchar* reading_end = input_ptr; // Pointeur actif

    // On récupère le header wav
    wavfile_header_t* header = (wavfile_header_t*)input_ptr;
    // Donnés utile pour le décodage
    int32_t sample_rate = header->SampleRate;
    int16_t n_channels = header->NumChannels;
    int32_t wav_header_size = sizeof(wavfile_header_t);

    // Initialise le décodeur
    OpusDecoder* decoder = NULL;
    int size = 0;
    size = opus_decoder_get_size(n_channels); /* max 2 channels */
    decoder = (OpusDecoder*)malloc(size);
    if (!decoder) {
      printf("fail to create encoder");
      return -1;
    }

    ret = opus_decoder_init(decoder, sample_rate, n_channels);
    if (ret != 0) {
      printf("fail to init %d", ret);
      return ret;
    }

    // Fichier de sortie qui va contenir le signal décodé (WAV).
    FILE* fp = fopen("decoded.wav", "w");
    if (!fp) {
        printf("fail to open file decoded.wav");
        return -1;
    }
    // On sauvegarde le header wav.
    fwrite(header, 1, wav_header_size, fp);
    reading_end += wav_header_size; // On skip le header pour débuter l'encodage
    opus_int32 chunk_size = 18;
    int frame_duration_ms = 5;
    int frame_size = sample_rate/1000*frame_duration_ms;
    opus_int16* buffer = (opus_int16*)malloc(frame_size*n_channels*sizeof(opus_int16));
    while(reading_end < input_ptr + file_size) {

        ret = opus_decode(decoder, reading_end, chunk_size, buffer, frame_size, 0);
        if (ret <= 0) {
            printf("fail to decode : %d\n", ret);
            break;
        }
        
        fwrite(buffer, 1, ret*2, fp);
        
        reading_end += chunk_size;
    }

    fclose(fp);
    munmap(input_ptr, file_size);
    free(decoder);

    printf("enc: done\n");

    return 0;
}
