/*
 * snd_unix.c
 *
 * UNIX(tm) sound interface (mainly OSS)
 */

/* $Id: snd_unix.c,v 1.4 2000/06/25 17:04:32 nyef Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ui.h"
#include "snd.h"
#include "unixdep.h"

#ifdef SYSTEM_LINUX
#include <sys/soundcard.h>
#define SOUND_DEVICE "/dev/dsp"
#endif

#ifdef SYSTEM_FREEBSD
#include <machine/soundcard.h>
#define SOUND_DEVICE "/dev/dsp"
#endif

#ifdef SYSTEM_NETBSD
#include <sys/audioio.h>
#define SOUND_DEVICE "/dev/sound"
#endif

#ifdef SYSTEM_OPENBSD
#error No sound support on OpenBSD systems yet
#endif

#include "pulse/simple.h"
static pa_simple *pulse_server=NULL;
#define DEFAULT_SAMPLE_RATE 48000


unsigned char final_wave[2048];
int waveptr;
int wavflag;
int sound_fd;

void snd_output_4_waves(int samples, u8 *wave1, u8 *wave2, u8 *wave3, u8 *wave4)
{
    int i;
    int error;

    if (pulse_server) {
	for (i = 0; i < samples; i++) {
            if (wave4[i] != 0) {
	    final_wave[waveptr] = (wave1[i] + wave2[i] +
				   wave3[i] + wave4[i]) >> 2;
            }else {
                 final_wave[waveptr] = (wave1[i] + wave2[i] +
				   wave3[i] ) >> 2;
            }
	    waveptr++;
	    if (waveptr == 2048) {
		waveptr = 0;
		wavflag = 2;
	    } else if (waveptr == 1024) {
		wavflag = 1;
	    }
	}
	
	if (wavflag) {
	    if (pa_simple_write(pulse_server, &final_wave[(wavflag - 1) << 10], 1024,&error)) {
		deb_printf("wrote less than 1024 bytes\n");
	    }
            
	    wavflag = 0;
       
	}
    }
}


void beep (int durationParm) {
          int error;
                  int sample = 261;
                  int frequency=261;
                  int duration=500;
                  
                  if (durationParm > 50) {
                      duration=durationParm;
                  }

                  int amplitude=16;

                  int half_period = (int) (sample/frequency/2);
                  int beep = ((char) (amplitude))*half_period+(char)(0)*half_period;

                  beep = (int) ((duration*frequency/1000))%255;
                  pa_simple_write(pulse_server,&beep,1,&error);

}


void snd_init(void)
{
         
}

#ifdef SYSTEM_NETBSD
int snd_open(int samples_per_sync, int sample_rate)
{
    int result;
    audio_info_t info;

    waveptr = 0;
    wavflag = 0;
    
    printf("opening "SOUND_DEVICE"...");
    sound_fd = open(SOUND_DEVICE, O_WRONLY);
    if (sound_fd < 0) {
	perror("failed");
	sound_fd = 0;
	return 0;
    } else {
	printf("done.\n");
    }

    AUDIO_INITINFO(&info);

    printf("setting unsigned 8-bit format...");
    info.play.encoding = AUDIO_ENCODING_ULINEAR;
    /* this doesn't work for me, so ignore results */
    result = ioctl(sound_fd, AUDIO_SETINFO, &info);
    if (result < 0)
    {
	perror("warning: unsigned linear mode failed, using signed");
	info.play.encoding = AUDIO_ENCODING_SLINEAR;
    }

    printf("setting sound rate to %dHz...", sample_rate);
    info.play.sample_rate = 48000;

    /* 8 bits per sample */
    info.play.precision = 8;

    printf("setting mono mode...");
    info.play.channels = 1;
    info.mode = AUMODE_PLAY_ALL;

    /* "frag size" */
    info.blocksize = 1 << 8;

    /* "number of frags", hiwater mark */
    info.hiwat = 8;

    result = ioctl(sound_fd, AUDIO_SETINFO, &info);
    if (result < 0) {
	perror("initializing sound failed");
	close(sound_fd);
	sound_fd = 0;
	return 0;
    } else {
	printf("done.\n");
    }
    
    return 1;
}

#else
int snd_open(int samples_per_sync, int sample_rate)
{
     
    waveptr = 0;
    wavflag = 0;
    
    int rate=sample_rate;
    pa_sample_spec sampleSpec;
    if (pulse_server != NULL) {
        pa_simple_free(pulse_server);
    }
    sampleSpec.rate=sample_rate;
    sampleSpec.format=PA_SAMPLE_U8;
    sampleSpec.channels=1; 
    pulse_server= pa_simple_new(NULL,"DARCNESJOE",PA_STREAM_PLAYBACK,NULL,"Audio",&sampleSpec,NULL,NULL,NULL);
    if (pulse_server== NULL) { 
         printf("ERROR OPENING PULSE");
    }
    return 1;
}
#endif

void snd_close(void)
{
    if (pulse_server) {
      pa_simple_free(pulse_server);
    }
}

/*
 * $Log: snd_unix.c,v $
 * Revision 1.4  2000/06/25 17:04:32  nyef
 * fixed to automatically detect system type
 *
 * Revision 1.3  2000/02/14 02:04:13  nyef
 * addded (untested) support for NetBSD
 *
 * Revision 1.2  1999/12/24 16:52:42  nyef
 * changed to compile clean on FreeBSD
 *
 * Revision 1.1  1999/10/31 02:37:58  nyef
 * Initial revision
 *
 */
