// This file is part of the RaspiVideoRecorder package.
// (c) Alexander Lukashevich <aleksandr.dwt@gmail.com>
// For the full copyright and license information, please view the LICENSE file that was distributed with this source code.

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <OMX_Component.h>
#include <bcm_host.h>
#include <ilclient.h>
#include <dirent.h>

int FPS = 25;

int compare_function(const void *a,const void *b) {
    const char* str1 = (const char*)a;
    const char* str2 = (const char*)b;

    int length[2];
    int number[2];

    int j = 0;
    for(; j < 2; j++) {
        char *str;
        if (j == 0) {
            asprintf(&str, "%s", str1);
        } else {
            asprintf(&str, "%s", str2);
        }

        char strStripped[200];
        char c1[1];

        int i = 0, c = 0;
        char *s = "_";
        for(; i < strlen(str); i++)
        {
            sprintf(c1, "%c",str[i]);

            if (strcmp("_", c1) != 0)
            {
                strStripped[c] = str[i];
                c++;
            }
        }
        strStripped[c] = '\0';

        length[j] = strlen(str);
        number[j] = atoi(strStripped);
    }

    if (number[0] == number[1]) {
        if (length[0] > length[1]) {
            return 1;
        } else {
            return -1;
        }
    } else {
        if (number[0] > number[1]) {
            return 1;
        } else {
            return -1;
        }
    }
}

void printState(OMX_HANDLETYPE handle) {
    // elided
}

char *err2str(int err) {
    return "elided";
}

void eos_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
//    fprintf(stderr, "Got eos event\n");
}

void error_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
 //   fprintf(stderr, "OMX error %s\n", err2str(data));
}

int get_file_size(char *fname) {
    struct stat st;

    if (stat(fname, &st) == -1) {
	perror("Stat'ing img file");
	return -1;
    }
    return(st.st_size);
}


OMX_ERRORTYPE read_into_buffer_and_empty(FILE *fp,
					 COMPONENT_T *component,
					 OMX_BUFFERHEADERTYPE *buff_header,
					 int *toread) {
    OMX_ERRORTYPE r;
    int buff_size = buff_header->nAllocLen;
    int nread = fread(buff_header->pBuffer, 1, buff_size, fp);

//    printf("Read %d\n", nread);

    buff_header->nFilledLen = nread;
    *toread -= nread;

    if (*toread <= 0) {
//		printf("Setting EOS on input\n");
		buff_header->nFlags |= OMX_BUFFERFLAG_EOS;
    }

    r = OMX_EmptyThisBuffer(ilclient_get_handle(component),			    buff_header);
    if (r != OMX_ErrorNone) {
		fprintf(stderr, "Empty buffer error %s\n",		err2str(r));
    }

    return r;
}




void setup_decodeComponent(
    ILCLIENT_T  *handle,
    char *decodeComponentName,
    COMPONENT_T **decodeComponent
) {
    int err;

    err = ilclient_create_component(handle,				    decodeComponent,				    decodeComponentName,				    ILCLIENT_DISABLE_ALL_PORTS				    |				    ILCLIENT_ENABLE_INPUT_BUFFERS				    );
    if (err == -1) {
	fprintf(stderr, "DecodeComponent create failed\n");
	exit(1);
    }
    printState(ilclient_get_handle(*decodeComponent));

    err = ilclient_change_component_state(*decodeComponent,
					  OMX_StateIdle);
    if (err < 0) {
	fprintf(stderr, "Couldn't change state to Idle\n");
	exit(1);
    }
    printState(ilclient_get_handle(*decodeComponent));

    // must be before we enable buffers
//    printf("Setting image decoder format\n");
    OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;

    memset(&imagePortFormat, 0, sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE));
    imagePortFormat.nSize = sizeof(OMX_IMAGE_PARAM_PORTFORMATTYPE);
    imagePortFormat.nVersion.nVersion = OMX_VERSION;

    imagePortFormat.nPortIndex = 320;
    imagePortFormat.eCompressionFormat = OMX_IMAGE_CodingJPEG;
    OMX_SetParameter(ilclient_get_handle(*decodeComponent),     OMX_IndexParamImagePortFormat, &imagePortFormat);
}




void setup_renderComponent(
    ILCLIENT_T  *handle,
    char *renderComponentName,
    COMPONENT_T **renderComponent
) {
    int err;

   err = ilclient_create_component(handle,                                renderComponent,                                renderComponentName,                                ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_OUTPUT_BUFFERS               );
    if (err == -1) {
	fprintf(stderr, "RenderComponent create failed\n");
	exit(1);
    }
    printState(ilclient_get_handle(*renderComponent));

    err = ilclient_change_component_state(*renderComponent,					  OMX_StateIdle);
    if (err < 0) {
	fprintf(stderr, "Couldn't change state to Idle\n");
	exit(1);
    }
    printState(ilclient_get_handle(*renderComponent));


	OMX_PARAM_PORTDEFINITIONTYPE rportdef;
    rportdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    rportdef.nVersion.nVersion = OMX_VERSION;
    rportdef.nPortIndex = 201;

    err = OMX_GetParameter(ilclient_get_handle(*renderComponent),
                           OMX_IndexParamPortDefinition, &rportdef);
    if (err != OMX_ErrorNone) {
        fprintf(stderr, "Error getting render port params %s\n", err2str(err));
		exit(1);
    }

    rportdef.format.video.xFramerate   = 25 << 16;   // WARNING! IT DOESN'T WORK - FPS is always 25 despite this parameter
    rportdef.format.video.eColorFormat = OMX_COLOR_FormatUnused;
    rportdef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    rportdef.format.video.nBitrate     = 8000000;

    err = OMX_SetParameter(ilclient_get_handle(*renderComponent),
                           OMX_IndexParamPortDefinition, &rportdef);
    if (err != OMX_ErrorNone) {
        fprintf(stderr, "Error setting render port params %s\n", err2str(err));
		exit(1);
    }
}



int main(int argc, char** argv) {

    if (argc != 3) {
        fprintf(stderr, "Specify input dir path and output file path\n");
        exit(1);
    }

    //************************************************
    //*********************** Open output file
    //************
	FILE *outf;
  	outf = fopen(argv[2], "w");
   if (outf == NULL) {
      fprintf(stderr, "Failed open file for writing video\n");
      exit(1);
   }


    //************************************************
    //*********************** Init hardware
    //************
    int i;
    char *decodeComponentName;
   char *renderComponentName;
    int err;
    ILCLIENT_T  *handle;
    COMPONENT_T *decodeComponent;
    COMPONENT_T *renderComponent;
    OMX_BUFFERHEADERTYPE *buff_header;

    decodeComponentName = "image_decode";
    renderComponentName = "video_encode";

    bcm_host_init();

    handle = ilclient_init();
    if (handle == NULL) {
        fprintf(stderr, "IL client init failed\n");
        exit(1);
    }

    if (OMX_Init() != OMX_ErrorNone) {
        ilclient_destroy(handle);
        fprintf(stderr, "OMX init failed\n");
        exit(1);
    }

    ilclient_set_error_callback(handle,				error_callback,				NULL);
    ilclient_set_eos_callback(handle,			      eos_callback,			      NULL);

    setup_decodeComponent(handle, decodeComponentName, &decodeComponent);
   setup_renderComponent(handle, renderComponentName, &renderComponent);

    // both components now in Idle state, no buffers, ports disabled
    // input port
    ilclient_enable_port_buffers(decodeComponent, 320, 				 NULL, NULL, NULL);
    ilclient_enable_port(decodeComponent, 320);

    err = ilclient_change_component_state(decodeComponent,					  OMX_StateExecuting);
    if (err < 0) {
        fprintf(stderr, "Couldn't change decode state to Executing\n");
        exit(1);
    }
    printState(ilclient_get_handle(decodeComponent));


    //************************************************
    //************ Get number of seconds
    //*********
    int numberOfSeconds = 0;
    char *dirPath;
    asprintf(&dirPath, "%s/", argv[1]);
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (dirPath)) == NULL) {
        fprintf(stderr, "Can not open input dir\n");
        exit(1);
    }

    while ((ent = readdir (dir)) != NULL) {
        char *fileName;
        asprintf(&fileName, "%s", ent->d_name);
        if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0) {
            numberOfSeconds++;
        }
    }
    closedir (dir);

    if (numberOfSeconds == 0) {
        fprintf(stderr, "No seconds to process\n");
        exit(1);
    }
//    printf("Number of seconds to process: %d\n", numberOfSeconds);


    //************************************************
    //************ Process seconds one by one
    //*********
    int fileSize = 0;
    FILE *fp;
    int toread;

    int second = 0;
    bool firstTime = true;

    while (true) {
        char *dirPath;
        asprintf(&dirPath, "%s/%d/", argv[1], second);
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir (dirPath)) == NULL) {
              break;
        }

        char filenames[400][FPS];
        int filesCount = 0;
        while ((ent = readdir (dir)) != NULL) {
            char *fileName;
            asprintf(&fileName, "%s", ent->d_name);
            if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0) {
                strcpy(filenames[filesCount++], fileName);
            }
        }
        closedir (dir);

        if (filesCount == 0) {
            continue;
        }
        qsort(filenames, filesCount, sizeof(filenames[0]), compare_function);

        int j = 0;
        for (; j < filesCount; j++) {
            char *fileFullPath;
            asprintf(&fileFullPath, "%s%s", dirPath, filenames[j]);
//            printf ("Process file: %s\n", fileFullPath);


            fp = fopen(fileFullPath, "r");

           if (fp == NULL) {
              fprintf(stderr, "Failed to open image to read: %s\n", fileFullPath);
              exit(1);
           }

            toread = get_file_size(fileFullPath);
            fileSize = toread;



        if (firstTime) {
                firstTime = false;

                // Read the first block so that the decodeComponent can get
                // the dimensions of the image and call port settings
                // changed on the output port to configure it
                buff_header = 		ilclient_get_input_buffer(decodeComponent,					  320,					  1 );

                if (buff_header != NULL) {
                    read_into_buffer_and_empty(fp,						   decodeComponent,						   buff_header,						   &toread);

                    // If all the file has been read in, then
                    // we have to re-read this first block.
                    // Broadcom bug?
                    if (toread <= 0) {
//                        printf("Rewinding\n");
                        // wind back to start and repeat
                        fp = freopen(fileFullPath, "r", fp);
                        toread = get_file_size(fileFullPath);
                    }
                }

                // wait for first input block to set params for output port
                ilclient_wait_for_event(decodeComponent, 					OMX_EventPortSettingsChanged, 					321, 0, 0, 1,					ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 					5);

//                printf("Port settings changed\n");


                TUNNEL_T tunnel;
                set_tunnel(&tunnel, decodeComponent, 321, renderComponent, 200);
                if ((err = ilclient_setup_tunnel(&tunnel, 0, 0)) < 0) {
                    fprintf(stderr, "Error setting up tunnel %X\n", err);
                    exit(1);
                } else {
//                    printf("Tunnel set up ok\n");
                }

               // Okay to go back to processing data
               // enable the decode output ports

               ilclient_enable_port(decodeComponent, 321);


              // enable the render output ports

                ilclient_enable_port_buffers(renderComponent, 201, 				 NULL, NULL, NULL);
                 ilclient_enable_port(renderComponent, 201);


                // set decoder only to executing state
                err = ilclient_change_component_state(renderComponent,						  OMX_StateExecuting);
                if (err < 0) {
                    fprintf(stderr, "Couldn't change state to Executing\n");
                    exit(1);
                }

        }


            // now work through the file
            while (toread > 0) {
                OMX_ERRORTYPE r;

                // do we have a decode input buffer we can fill and empty?
                buff_header = 			ilclient_get_input_buffer(decodeComponent,						  320,						  1 /* block */);
                if (buff_header != NULL) {
                    read_into_buffer_and_empty(fp,						   decodeComponent,						   buff_header,						   &toread);
                }

                // do we have an output buffer that has been filled?
                buff_header = 			ilclient_get_output_buffer(renderComponent,						  201,						  0 /* no block */);
                if (buff_header != NULL) {
//                    printf("Write output to file %d...\n", buff_header->nFilledLen);
                    fwrite(buff_header->pBuffer, 1, buff_header->nFilledLen, outf);

                    if (buff_header->nFlags & OMX_BUFFERFLAG_EOS) {
//                        printf("Got EOS, but very early!\n");
                    }

                    OMX_FillThisBuffer(ilclient_get_handle(renderComponent), 					   buff_header);
                }

            }


            int done = 0;
            while ( !done ) {
//                printf("Getting last output buffers\n");
                buff_header = 			ilclient_get_output_buffer(renderComponent,						   201,						   1 /* block */);

                if (buff_header != NULL) {
//                    printf("Write output to file %d...\n", buff_header->nFilledLen);
                    fwrite(buff_header->pBuffer, 1, buff_header->nFilledLen, outf);

                    if (buff_header->nFlags & OMX_BUFFERFLAG_EOS) {
//                        printf("Got EOS\n");
                        done = 1;
                    }
                }

                OMX_FillThisBuffer(ilclient_get_handle(renderComponent), 					   buff_header);
           }


//            printf("One more file has been processed\n");
            fclose(fp);
        }

        second++;

        printf("%d%%\n", second * 100 / numberOfSeconds);
    }

	fclose(outf);

	exit(0);
}
