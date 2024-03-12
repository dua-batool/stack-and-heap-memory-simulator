#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MEMSIZE 500
#define MAX_INPUT_SIZE 100
int top_of_stack = 395;

typedef struct framestatus {
	int number;               // frame number
    char name[8];             // function name representing the frame
	int functionaddress;      // address of function in code section (will be randomly generated in this case)
	int frameaddress;         // starting address of frame belonging to this header in Stack
	char used;                // a boolean value indicating wheter the frame status entry is in use or not
} framestatus; // total 21 bytes

typedef struct freelist {
	int start;         // start address of free region
	int size;                 // size of free region
	struct freelist * next;   // pointer to next free region
} freelist; // total 8 bytes

typedef struct all_frames {
	framestatus frames[5];      
} all_frames;

typedef struct allocated {
	char name[8];
	int address;
	struct allocated * next;
} allocated;

//helper functions
// to initialize the values of all the frames
void SetNull(all_frames * fs, char memory[], char occupied[]){
	for (int i = 0; i < 5; i++) {
		fs->frames[i].number = -1;
		strcpy(fs->frames[i].name,"");
		fs->frames[i].functionaddress = -1;
		fs->frames[i].frameaddress = -1;
		fs->frames[i].used = '0';
	}

	for (int i = 0; i < MEMSIZE -1; i++) {
		memory[i] = '\0';
		occupied[i] = '\0';
	}
}

// returns the index for a new frame
int GetFrame(all_frames * fs){
	for (int i = 0; i < 5; i++) {
		if (fs->frames[i].used == '0') {
			return i;
		}
	}
	return -1;     // to indicate that all of the 5 frames are in use
}

// returns the index of the current frame
int CurrentFrame(all_frames * fs){
	for (int i = 0; i < 5; i++) {
		if (fs->frames[i].used == '0') {
			return i-1;
		}
	}
	return 4;     // all 5 frames are in use so returning the last frame to be deleted
}

//Functions
// creates a new frame in stack
void CreateFrame(all_frames * fs, int * currentstacksize, char name[], int address){
	for (int i = 0; i < 5; i++){
		if(strcmp(fs->frames[i].name, name) == 0){
			fprintf(stderr, "Function already exists.\n");
			return;
		}
	}

	int index = GetFrame(fs);
	if (index == -2) {
		fprintf(stderr, "Cannot create another frame, maximum number of frames have reached.\n");
		return;
	} 

	int size = MEMSIZE - top_of_stack;   
	if (size > *(currentstacksize) -10) {
		if ((*currentstacksize += 10) >= 245 ) {
			fprintf(stderr, "Stack overflow, not enough memory available for new function.\n”");
			return;
		} else *currentstacksize += 10;
	} 

	fs->frames[index].number = index;
	strcpy(fs->frames[index].name, name);     
	fs->frames[index].functionaddress = address;
	fs->frames[index].frameaddress = top_of_stack -1;
	fs->frames[index].used = '1';
	top_of_stack -= 10;
	printf("frame created.\n");
}

// deletes frame on the top of the stack
void DeleteFrame(all_frames * fs, char memory[], char occupied[], allocated * allocated_region){
	int index = CurrentFrame(fs);
	if (index == -1) {
		fprintf(stderr, "Stack is empty.\n");
		return;
	}

	char * casted;
	for (int i = fs->frames[index].frameaddress; i >= top_of_stack; i--) {
		casted = (char*)&memory[i];
		*casted = '\0';
		occupied[i] = '\0';
	}

	top_of_stack = fs->frames[index].frameaddress + 1;
	fs->frames[index].number = -1;
	strcpy(fs->frames[index].name, "");     
	fs->frames[index].functionaddress = -1;
	fs->frames[index].frameaddress = -1;
	fs->frames[index].used = '0';
}

// creates an integer variable on the current frame in stack
void CreateIntVar(all_frames * fs, int * currentstacksize, char memory[], char occupied[], char name[], int value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		fprintf(stderr, "Cannot create the variable as stack is empty. Create a Frame first.\n");
		return;
	}

	char data_inserted = '0';
	int *casted;
	int framesize = top_of_stack - fs->frames[index].frameaddress;

	for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
		if (occupied[i] == '\0' && i - top_of_stack >= 4) {
			casted = (int *)&memory[i-3];
			*casted = value;
			for (int j = 0; j < 4; j++) {
				occupied[i-j] = '1';
			}
			printf("%d   %d \n", i-3, memory[i-3]);
			data_inserted = '1';
			break;
		}
	}

	if (framesize > 77) {
		fprintf(stderr, "The frame is full, cannot create more data on it.");
		return;
	}

	if (data_inserted == '0') {
		int size = MEMSIZE - top_of_stack;   
		if (size > *(currentstacksize) -10) {
			if ((*currentstacksize += 10) >= 245 ) {
				fprintf(stderr, "Stack overflow, not enough memory available.\n”");
				return;
			} else *currentstacksize += 10;
		} 
		top_of_stack -= 10;
		for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
			if (occupied[i] == '\0') {
				casted = (int *)&memory[i-3];
				*casted = value;
				for (int j = 0; j < 4; j++) {
					occupied[i-j] = '1';
				}
				printf("%d   %d \n", i-3, memory[i-3]);
					data_inserted = '1';
				break;
			}
		}
	}

	
}

// creates a double variable on the current frame in stack
void CreateDoubleVar(all_frames * fs, int * currentstacksize, char memory[], char occupied[], char name[], double value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		fprintf(stderr, "Cannot create the variable as stack is empty. Create a Frame first.\n");
		return;
	}

	char data_inserted = '0';
	double *casted;
	

	for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
		if (occupied[i] == '\0' && i - top_of_stack >= 8) {
			casted = (double *)&memory[i-7];
			*casted = value;
			for (int j = 0; j < 8; j++) {
				occupied[i-j] = '2';
			}
			printf("%d   %f \n", i-7, *((double*)&memory[i-7]));
			data_inserted = '1';
			break;
		}
	}

	int framesize = top_of_stack - fs->frames[index].frameaddress;
	if (framesize > 73) {
		fprintf(stderr, "The frame is full, cannot create more data on it.");
		return;
	}

	if (data_inserted == '0') {
		int size = MEMSIZE - top_of_stack;   
		if (size > *(currentstacksize) -10) {
			if ((*currentstacksize += 10) >= 245 ) {
				fprintf(stderr, "Stack overflow, not enough memory available.\n”");
				return;
			} else *currentstacksize += 10;
		} 
		top_of_stack -= 10;
		for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
			if (occupied[i] == '\0') {
				casted = (double *)&memory[i-7];
				*casted = value;
				for (int j = 0; j < 8; j++) {
					occupied[i-j] = '2';
				}
				printf("%d   %f \n", i-7, *((double*)&memory[i-7]));
				data_inserted = '1';
				break;
			}
		}
	}

}

// creates a character variable on the current frame in stack
void CreateCharVar(all_frames * fs, int * currentstacksize, char memory[], char occupied[], char name[], char value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		fprintf(stderr, "Cannot create the variable as stack is empty. Create a Frame first.\n");
		return;
	}

	char data_inserted = '0';

	for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
		if (occupied[i] == '\0' && i - top_of_stack >= 1) {
			memory[i] = value;
			occupied[i] = '3';
			printf("%d   %c \n", i, memory[i]);
			data_inserted = '1';
			break;
		}
	}

	int framesize = top_of_stack - fs->frames[index].frameaddress;
	if (framesize > 79) {
		fprintf(stderr, "The frame is full, cannot create more data on it.");
		return;
	}

	if (data_inserted == '0') {
		int size = MEMSIZE - top_of_stack;   
		if (size > *(currentstacksize) -10) {
			if ((*currentstacksize += 10) >= 245 ) {
				fprintf(stderr, "Stack overflow, not enough memory available for new function.\n”");
				return;
			} else *currentstacksize += 10;
		} 
		top_of_stack -= 10;
		for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
			if (occupied[i] == '\0') {
				memory[i] = value;
				occupied[i] = '3';
				printf("%d   %c \n", i, memory[i]);
				data_inserted = '1';
				break;
			}
		}
	}
}

// allocates a buffer on heap
void CreateBuffer(all_frames * fs, char memory[], char occupied[], int * heapsize, allocated * new_node, allocated * allocated_region, freelist * free_region, char buffername[], int buffersize){
	int index = CurrentFrame(fs);
	if (index == -1) {
		fprintf(stderr, "There is no frame on stack. Create a frame first to create buffer.\n");
		return;
	}

	int framesize = fs->frames[index].frameaddress - top_of_stack;
	if (framesize + 4 > 80) {
		fprintf(stderr, "Not enough space on the current frame to allocate a buffer on heap. Create a new frame first.\n");
		return;
	}
	if (fs->frames[index].frameaddress - framesize -4 < top_of_stack) { 
		top_of_stack -= 10;
	}
	int * casted;

	freelist * current_free = free_region;
	while (current_free->next != NULL) {
		current_free = current_free->next;
	}

	if (current_free->size < buffersize + 8) {
		if (*heapsize + buffersize + 8 < 255) {
			*heapsize = *heapsize + buffersize + 8;
			current_free->size = *heapsize - current_free->start;
		}
	}

	allocated * current_allocated = allocated_region;

	int heap_address;
	for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
		if (occupied[i] == '\0' && framesize < 77) {
			for (int j = 0; j < 4; j++){
				occupied[i-j] = '4';
			}
			casted = (int*)&memory[i-3];
			*casted = current_free->start;
			heap_address = *casted;
			for (int k = heap_address; k < heap_address + buffersize + 8; k++) {
				if (k >= heap_address + 8) {
					occupied[k] = '5';
				} else {
					occupied[k] = '4';
				}	
			}

			if (current_allocated->address == -1) {
				strcpy(current_allocated->name,buffername);
				current_allocated->address = i-3;
				current_allocated->next = NULL;
				printf("\nAllocated region name: %s\n", current_allocated->name);
				printf("Allocated region address: %d\n", current_allocated->address);
			} else {
				while (current_allocated->next != NULL) {
					current_allocated = current_allocated->next;
				}
				strcpy(new_node->name,buffername);
				new_node->address = i-3;
				new_node->next = NULL;
				current_allocated->next = new_node;
				printf("\nAllocated region name: %s\n", allocated_region->next->name);
				printf("Allocated region address: %d\n", new_node->address);
			}

			current_free->start += buffersize + 8;
			current_free->size -= (buffersize + 8);
			printf("Free region start: %d\n", current_free->start);
			printf("Free region size: %d\n", current_free->size);
			break;
		}
	}

	heap_address = *casted;
	printf("heap address: %d\n", heap_address);
	char alphabets[26];
	for (int u = 0; u < 26; u++) {
		alphabets[u] = (char)'a' + u;
	}

	printf("\nprinting buffer contents:\n");
	for (int x = heap_address + 8; x < buffersize + heap_address + 8; x++) {
		memory[x] = alphabets[x%26];
		printf("%c", memory[x]);
	}
	printf("\n\n");
}

// deallocates the buffer on heap
void RemoveBuffer(all_frames * fs, char memory[], char occupied[], int * currentheapsize, freelist * free_node, allocated * allocated_region, freelist * free_region, char name[]){
	allocated * current_allocated = allocated_region;
	allocated * previous_node = current_allocated;
	int address;
	int found = 0;
	int endaddress;

	while (current_allocated != NULL) {
		
		if (strcmp(current_allocated->name, name) == 0) {
			address = current_allocated->address;
			if (current_allocated->next == NULL) {
				int i = memory[address];
				while(occupied[i] != '\0') {
					memory[i] = '\0';
					occupied[i] = '\0';
					i++;
				}
			} else {
				endaddress = current_allocated->next->address;
				for (int i = memory[address]; i < memory[address] + 8 + memory[endaddress]; i++) {
					memory[i] = '\0';
					occupied[i] = '\0';
				}
			}
			found = 1;
			break;
		} else {
			previous_node = current_allocated;
			current_allocated = current_allocated->next;
		} 
	}

	if (found == 0) {
		fprintf(stderr, "Error: Buffer not found.\n");
		return;
	}

	if (current_allocated->next == NULL) {
		free_region->start = memory[address];
		if (current_allocated == previous_node) {
			strcpy(current_allocated->name, "");
			current_allocated->address = -1;
			current_allocated->next = NULL;
		} else previous_node->next = NULL;
	} else {
		free_node->start = free_region->start;
		free_node->size = free_region->size;
		free_node->next = free_region->next;

		free_region->start = memory[address];
		free_region->size = memory[current_allocated->next->address];
		free_region->next = free_node;
		if (previous_node == current_allocated) {
			current_allocated = current_allocated->next;
		} else {
			previous_node->next = current_allocated->next;
		}
	}
}

// shows the state of the memory
void ShowMemory(all_frames * fs, char memory[], char occupied[], int currentstacksize, int currentheapsize){
	printf("\nShowing Memory:\n");
	printf("********************* Frame List (500-395) **************************\n");
	for (int k = 0; k < 5; k++) {
		if (fs->frames[k].used == '1') {
			printf("Frame %d:\n", k);
			printf("Frame name: %s\n", fs->frames[k].name);
			printf("Frame address: %d\n", fs->frames[k].frameaddress);
			printf("Function address: %d\n", fs->frames[k].functionaddress);
		}
	}
	printf("********************* Stack Frames (395-%d) ************************\n", currentstacksize);
	int i = 394;
	while (i > 255) {
		if (occupied[i] == '1') {
			printf("Integer value at Memory Address %d is:        %d\n", i-3, memory[i-3]);
			i -= 4;
		} else if (occupied[i] == '2') {
			printf("Double value at Memory Address %d is:         %f\n", i-7, *((double*)&memory[i-7]));
			i -= 8;
		} else if (occupied[i] == '3') {
			printf("Character value at Memory Address %d is:      %c\n", i, memory[i]);
			i -= 1;
		} else if (occupied[i] == '4') {
			printf("Heap Address value at Memory Address %d is:   %d\n", i-3, memory[i-3]);
			i -= 4;
		} else {
			i--;
		}
	}

	printf("********************* Heap Frames (0-%d) ***************************\n", currentheapsize);
	int j = 0;
	while (j < 255) {
		if (occupied[j] == '5') {
			printf("%c", memory[j]);
			j += 1;
		} else if (occupied[j] == '4') {
			printf(" ");
			j += 1;
		} else {
			j += 1;
		}
	}
	printf("\n");
}

int main (int argc, char * argv[]) {
	int currentstacksize = 200;  
	int currentheapsize = 100;
	char memory[MEMSIZE];         // Buffer that will emulate stack and heap memory
	char occupied[MEMSIZE];

	all_frames fs;

	freelist * free_region;
	freelist freenode;
	free_region = &freenode;
	free_region->start = 0;
	free_region->size = 100;
	free_region->next = NULL;

	allocated * allocated_region; 
	allocated allocatednode;
	allocated_region = &allocatednode;  
	strcpy(allocated_region->name, "");
	allocated_region->address = -1;
	allocated_region->next = NULL;

	allocated * new_node;
	allocated node;
	new_node = &node;
	freelist * free_node;
	freelist freee;
	free_node = &freee;

	SetNull(&fs, memory, occupied);
	all_frames * casted;


	while (1) {
        printf("prompt: ");
        char input[100];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';  // adding null character

        if (strcmp(input, "exit") == 0) {    // exiting if the user enters 'exit'
            exit(0); 
        }

        char *myargs[100];
        int count = 0;
        char *token = strtok(input, " ");

        while (token != NULL) {
            myargs[count] = strdup(token);
            count++;
            token = strtok(NULL, " ");
        }
        myargs[count] = NULL;

        // Check the command and call the appropriate function
        if (strcmp(myargs[0], "CF") == 0) {
            if (count == 3) {
                CreateFrame(&fs, &currentstacksize, myargs[1], atoi(myargs[2]));
            } else {
                fprintf(stderr, "Error: CF command requires two arguments\n");
            }
        } else if (strcmp(myargs[0], "DF") == 0) {
			if (count == 1) {
				DeleteFrame(&fs, memory, occupied, allocated_region);
			} else {
				fprintf(stderr, "Error: DF command does not require any argument\n");
			}
        } else if (strcmp(myargs[0], "CI") == 0) {
            if (count == 3) {
                CreateIntVar(&fs, &currentstacksize, memory, occupied, myargs[1], atoi(myargs[2]));
            } else {
                fprintf(stderr, "Error: CI command requires two arguments\n");
            }
        } else if (strcmp(myargs[0], "CD") == 0) {
            if (count == 3) {
                CreateDoubleVar(&fs, &currentstacksize, memory, occupied, myargs[1], strtod(myargs[2], NULL));
            } else {
                fprintf(stderr, "Error: CD command requires two arguments\n");
            }
        } else if (strcmp(myargs[0], "CC") == 0) {
            if (count == 3) {
                CreateCharVar(&fs, &currentstacksize, memory, occupied, myargs[1], myargs[2][0]);
            } else {
                fprintf(stderr, "Error: CC command requires two arguments\n");
            }
        } else if (strcmp(myargs[0], "CH") == 0) {
            if (count == 3) {
                CreateBuffer(&fs, memory, occupied, &currentheapsize, new_node, allocated_region, free_region, myargs[1], atoi(myargs[2]));
            } else {
                fprintf(stderr, "Error: CH command requires two arguments\n");
            }
        } else if (strcmp(myargs[0], "DH") == 0) {
            if (count == 2) {
                RemoveBuffer(&fs, memory, occupied, &currentheapsize, free_node, allocated_region, free_region, myargs[1]);
            } else {
                fprintf(stderr, "Error: DH command requires one argument\n");
            }
        } else if (strcmp(myargs[0], "SM") == 0) {
			if (count == 1){
				ShowMemory(&fs, memory, occupied, currentstacksize, currentheapsize);
			} else {
				fprintf(stderr, "Error: SM command does not require any argument\n");
			}
            
        } else {
			fprintf(stderr, "Error: Invalid input.\n");
		}

		casted = (all_frames*)&memory[395];
		*casted = fs;

        // Freeing the allocated memory before the next iteration
        for (int i = 0; i < count; i++) {
            free(myargs[i]);
        }
    }
	return 0;
}
