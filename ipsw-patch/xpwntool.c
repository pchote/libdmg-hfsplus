#include <stdio.h>
#include <string.h>
#include "xpwn/libxpwn.h"
#include "xpwn/nor_files.h"

#define BUFFERSIZE (1024*1024)

int main(int argc, char* argv[]) {
	char* inData;
	size_t inDataSize;
	init_libxpwn();

	if(argc < 3) {
		printf("usage: %s <infile> <outfile> [-t <template> [-c <certificate>]] [-k <key>] [-iv <key>]\n", argv[0]);
		return 0;
	}

	AbstractFile* template = NULL;
	AbstractFile* certificate = NULL;
	unsigned int* key = NULL;
	unsigned int* iv = NULL;
	int hasKey = FALSE;
	int hasIV = FALSE;

	int argNo = 3;
	while(argNo < argc) {
		if(strcmp(argv[argNo], "-t") == 0 && (argNo + 1) < argc) {
			template = createAbstractFileFromFile(fopen(argv[argNo + 1], "rb"));
			if(!template) {
				fprintf(stderr, "error: cannot open template\n");
				return 1;
			}
		}

		if(strcmp(argv[argNo], "-c") == 0 && (argNo + 1) < argc) {
			certificate = createAbstractFileFromFile(fopen(argv[argNo + 1], "rb"));
			if(!certificate) {
				fprintf(stderr, "error: cannot open template\n");
				return 1;
			}
		}

		if(strcmp(argv[argNo], "-k") == 0 && (argNo + 1) < argc) {
			size_t bytes;
			hexToInts(argv[argNo + 1], &key, &bytes);
			hasKey = TRUE;
		}

		if(strcmp(argv[argNo], "-iv") == 0 && (argNo + 1) < argc) {
			size_t bytes;
			hexToInts(argv[argNo + 1], &iv, &bytes);
			hasIV = TRUE;
		}

		argNo++;
	}

	AbstractFile* inFile;
	if(hasKey) {
		inFile = openAbstractFile2(createAbstractFileFromFile(fopen(argv[1], "rb")), key, iv);
	} else {
		inFile = openAbstractFile(createAbstractFileFromFile(fopen(argv[1], "rb")));
	}
	if(!inFile) {
		fprintf(stderr, "error: cannot open infile\n");
		return 2;
	}

	AbstractFile* outFile = createAbstractFileFromFile(fopen(argv[2], "wb"));
	if(!outFile) {
		fprintf(stderr, "error: cannot open outfile\n");
		return 3;
	}


	AbstractFile* newFile;

	if(template) {
		if(hasKey) {
			newFile = duplicateAbstractFile2(template, outFile, key, iv, certificate);
		} else {
			newFile = duplicateAbstractFile2(template, outFile, NULL, NULL, certificate);
		}
		if(!newFile) {
			fprintf(stderr, "error: cannot duplicate file from provided template\n");
			return 4;
		}
	} else {
		newFile = outFile;
	}

	if(hasKey) {
		if(newFile->type == AbstractFileTypeImg3) {
			AbstractFile2* abstractFile2 = (AbstractFile2*) newFile;
			abstractFile2->setKey(abstractFile2, key, iv);
		}
	}

	inDataSize = (size_t) inFile->getLength(inFile);
	inData = (char*) malloc(inDataSize);
	inFile->read(inFile, inData, inDataSize);
	inFile->close(inFile);

	newFile->write(newFile, inData, inDataSize);
	newFile->close(newFile);

	free(inData);

	if(key)
		free(key);

	if(iv)
		free(iv);

	return 0;
}

