// Exemple pour encoder un fichier WAV avec Opus.
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
#include <time.h>

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
uchar* map_file(const char* fn) {
    FILE* fp = fopen(fn, "r");
    if(!fp) {
        printf("fail to open file: %s", fn);
        return (uchar*)MAP_FAILED;
    }

    fseek(fp, 0L, SEEK_END);
    long pos = ftell(fp);
    uchar* ptr = (uchar*)mmap(NULL, pos, PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(fp), 0);

    fclose(fp);
    return ptr;
}

int main(int argc, char *argv[])
{
   
    int ret;

    uchar* input_ptr = map_file(argv[1]); // Pointeur de début de fichier
    if (input_ptr == MAP_FAILED) {
        printf("fail to map file");
        return -1;
    }
    int16_t* reading_end = (int16_t*)input_ptr; // Pointeur actif

    // On s'assure que le fichier est bien un fichier audio .wav
    wavfile_header_t* header = (wavfile_header_t*)input_ptr;
    char wav_marker[5] = "RIFF";
    if(strncmp(header->ChunkID, wav_marker, 4) != 0){
        printf("The input file is not a wav file..");
        return -1;
    }

    // Donnés utile pour l'encodage
    int32_t sample_rate = header->SampleRate;
    int16_t n_channels = header->NumChannels;
    int32_t audio_size = header->Subchunk2Size;
    int32_t wav_header_size = sizeof(wavfile_header_t);
    int32_t total_file_size = wav_header_size + audio_size;

    // Initialisation de l'encodeur Opus
    OpusEncoder *encoder = NULL;
    int size = 0;
    size = opus_encoder_get_size(n_channels);
    encoder = (OpusEncoder*)malloc(size);
    if (!encoder) {
      printf("fail to create encoder");
      return -1;
    }
    // Ici on détermine le sample rate, le nombre de channel et un option pour la méthode d'encodage.
    // OPUS_APPLICATION_RESTRICTED_LOWDELAY nous donne la plus faible latence, mais vous pouvez essayer d'autres options.
    ret = opus_encoder_init(encoder, sample_rate, n_channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY);
    if (ret != 0) {
      printf("fail to init %d", ret);
      return ret;
    }

    /* paramètres de l'encodeur */
    // Bitrate du signal encodé : Vous devez trouver le meilleur compromis qualité/transmission.
    ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(28000));
    if (ret != 0) {
        printf("fail to set bitrate");
        return ret;
    }

    // Complexité de l'encodage, permet un compromis qualité/latence
    ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));
    if (ret != 0) {
      printf("fail to set complexity");
      return ret;
    }

    // Variation du bitrate.
    ret = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
    if (ret != 0) {
      printf("fail to set vbr");
      return ret;
    }
    // type de signal à encoder (dans notre cas il s'agit de la musique)
    ret = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    if (ret != 0) {
      printf("fail to set signal");
      return ret;
    }

    // Fichier de sortie qui va contenir le signal encodé.
    FILE* fp = fopen("out.opus", "w");
    if (!fp) {
        printf("fail to open file out.opus");
        return -1;
    }
    // On sauvegarde le header wav simplement pour le réutiliser lors du décodage.
    fwrite(header, 1, wav_header_size, fp);
    reading_end += wav_header_size;  // On skip le header pour débuter l'encodage

    int max_data_size = 100;  // Nombre maximal de byte encodé par frame
    uchar* buffer = (uchar*)malloc(max_data_size);
    int frame_duration_ms = 5; // Une durée plus grande que 10 ms va introduire énormément de bruit dans un signal musical.
    int frame_size = sample_rate/1000*frame_duration_ms;
    clock_t start_time = clock();
    while((uchar*)reading_end < input_ptr + total_file_size) {
        ret = opus_encode(encoder, (opus_int16*)reading_end, frame_size, buffer, max_data_size);
        if (ret <= 0) {
            printf("fail to encode");
            break;
        }
        // Écriture du signal encodé dans le fichier
        fwrite(buffer, 1, ret, fp);
        
        // Incrémente le pointeur
        reading_end += frame_size;
    }
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("encoding opus done in %f sec\n", time_spent);

    fclose(fp);
    munmap(input_ptr, total_file_size);
    free(encoder);

    return 0;
}
