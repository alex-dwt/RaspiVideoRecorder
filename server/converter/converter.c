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

int filesCount = 0;
int currentFileId = 0;
int fileSize = 0;

void printState(OMX_HANDLETYPE handle) {
    // elided
}

char *err2str(int err) {
    return "elided";
}

void eos_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
    fprintf(stderr, "Got eos event\n");
}

void error_callback(void *userdata, COMPONENT_T *comp, OMX_U32 data) {
    fprintf(stderr, "OMX error %s\n", err2str(data));
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

    printf("Read %d\n", nread);

    buff_header->nFilledLen = nread;
    *toread -= nread;

    if (*toread <= 0) {
		printf("Setting EOS on input\n");
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
    printf("Setting image decoder format\n");
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

    rportdef.format.video.xFramerate   = 25 << 16;   // WARNING! IT DOESN WORK - FPS is alwas 25 despite this parameter 
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

	char filename[200];
	char filenames[15000][200];

	while(fgets(filename, sizeof filename, stdin)!=NULL) {
		strtok(filename, "\n");
		strcpy(filenames[filesCount++], filename);
	}

   if (filesCount == 0) {
      printf("No files\n");
      exit(1);
   }

	FILE *outf;

  	outf = fopen("/tmp/images/test.avi", "w");

   if (outf == NULL) {
      printf("Failed to open for writing video\n");
      exit(1);
   }

    int i;
    char *decodeComponentName;
   char *renderComponentName;
    int err;
    ILCLIENT_T  *handle;
    COMPONENT_T *decodeComponent;
    COMPONENT_T *renderComponent;

    FILE *fp;
    int toread; 
  
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


	for(currentFileId = 0; currentFileId < filesCount; ) {

		fp = fopen(filenames[currentFileId], "r");

	   if (fp == NULL) {
		  printf("Failed to open image to read\n");
		  exit(1);
	   }

		toread = get_file_size(filenames[currentFileId]);
		fileSize = toread;



	if (currentFileId == 0) {
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
					printf("Rewinding\n");
					// wind back to start and repeat
					fp = freopen(filenames[currentFileId], "r", fp);
					toread = get_file_size(filenames[currentFileId]);
				}
			}

			// wait for first input block to set params for output port
			ilclient_wait_for_event(decodeComponent, 					OMX_EventPortSettingsChanged, 					321, 0, 0, 1,					ILCLIENT_EVENT_ERROR | ILCLIENT_PARAMETER_CHANGED, 					5);

			printf("Port settings changed\n");


			TUNNEL_T tunnel;
			set_tunnel(&tunnel, decodeComponent, 321, renderComponent, 200);
			if ((err = ilclient_setup_tunnel(&tunnel, 0, 0)) < 0) {
				fprintf(stderr, "Error setting up tunnel %X\n", err);
				exit(1);
			} else {
				printf("Tunnel set up ok\n");
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
				printf("Write output to file %d...\n", buff_header->nFilledLen);
				fwrite(buff_header->pBuffer, 1, buff_header->nFilledLen, outf);

				if (buff_header->nFlags & OMX_BUFFERFLAG_EOS) {
					printf("Got EOS, but very early!\n");
				}

				OMX_FillThisBuffer(ilclient_get_handle(renderComponent), 					   buff_header); 
			}

		}


		int done = 0;
		while ( !done ) {
			printf("Getting last output buffers\n");
			buff_header = 			ilclient_get_output_buffer(renderComponent,						   201,						   1 /* block */);

			if (buff_header != NULL) {
				printf("Write output to file %d...\n", buff_header->nFilledLen);
				fwrite(buff_header->pBuffer, 1, buff_header->nFilledLen, outf);
			
				if (buff_header->nFlags & OMX_BUFFERFLAG_EOS) {
					printf("Got EOS\n");
					done = 1;
				}
			}

			OMX_FillThisBuffer(ilclient_get_handle(renderComponent), 					   buff_header); 
	   }

		
		printf("One more file has been processed\n");
		currentFileId++;
		fclose(fp);
	}

	fclose(outf);

    exit(0);
}
