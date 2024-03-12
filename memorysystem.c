#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	int next;
} allocated;

//helper functions
// to initialize the values of all the frames
void SetNull(all_frames * fs){
	for (int i = 0; i < 5; i++) {
		fs->frames[i].number = -1;
		strcpy(fs->frames[i].name,"");
		fs->frames[i].functionaddress = -1;
		fs->frames[i].frameaddress = -1;
		fs->frames[i].used = '0';
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
			printf("Function already exists.\n");
			exit(1);
		}
	}

	int index = GetFrame(fs);
	if (index == -2) {
		printf("Cannot create another frame, maximum number of frames have reached.\n");
		exit(1);
	} 

	int size = MEMSIZE - top_of_stack;   
	if (size > *(currentstacksize) -10) *currentstacksize += 10;

	fs->frames[index].number = index;
	strcpy(fs->frames[index].name, name);     
	fs->frames[index].functionaddress = address;
	fs->frames[index].frameaddress = top_of_stack -1;
	fs->frames[index].used = '1';
	top_of_stack -= 10;
}

// deletes frame on the top of the stack
void DeleteFrame(all_frames * fs, char memory[]){
	int index = CurrentFrame(fs);
	if (index == -1) {
		printf("Stack is empty.\n");
		exit(1);
	}

	char * casted;
	for (int i = fs->frames[index].frameaddress; i >= top_of_stack; i--) {
		casted = (char*)&memory[i];
		*casted = '\0';
	}

	top_of_stack = fs->frames[index].frameaddress + 1;
	fs->frames[index].number = -1;
	strcpy(fs->frames[index].name, "");     
	fs->frames[index].functionaddress = -1;
	fs->frames[index].frameaddress = -1;
	fs->frames[index].used = '0';
}

// creates an integer variable on the current frame in stack
void CreateIntVar(all_frames * fs, char memory[], char occupied[], char name[], int value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		printf("Cannot create the variable as stack is empty. Create a Frame first.\n");
		exit(1);
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
		printf("The frame is full, cannot create more data on it.");
		exit(1);
	}

	if (data_inserted == '0') {
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
void CreateDoubleVar(all_frames * fs, char memory[], char occupied[], char name[], double value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		printf("Cannot create the variable as stack is empty. Create a Frame first.\n");
		exit(1);
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
		printf("The frame is full, cannot create more data on it.");
		exit(1);
	}

	if (data_inserted == '0') {
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
void CreateCharVar(all_frames * fs, char memory[], char occupied[], char name[], char value) {
	int index = CurrentFrame(fs);
	if (index == -1) {
		printf("Cannot create the variable as stack is empty. Create a Frame first.\n");
		exit(1);
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
		printf("The frame is full, cannot create more data on it.");
		exit(1);
	}

	if (data_inserted == '0') {
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
void CreateBuffer(all_frames * fs, char memory[], char occupied[], int heapsize, allocated * allocated_region, freelist * free_region, char buffername[], int buffersize){
	int index = CurrentFrame(fs);
	if (index == -1) {
		printf("There is no frame on stack. Create a frame first to create buffer.\n");
		exit(1);
	}

	int framesize = fs->frames[index].frameaddress - top_of_stack;
	if (framesize + 4 > 80) {
		printf("Not enough space on the current frame to allocate a buffer on heap. Create a new frame first.\n");
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

	allocated * current_allocated = allocated_region;
	while (current_allocated->next != -1) {
		current_allocated = current_allocated->next;
	}
	int heap_address;
	for (int i = fs->frames[index].frameaddress; i > top_of_stack; i--) {
		if (occupied[i] == '\0' && framesize < 77) {
			for (int j = 0; j < 4; j++){
				occupied[i-j] = '4';
			}
			casted = (int*)&memory[i-3];
			*casted = current_free->start;
			heap_address = *casted;
			for (int k = heap_address; k < buffersize + 8; k++) {
				if (k >= heap_address + 8) {
					occupied[k] = '5';
				} else {
					occupied[k] = '4';
				}	
			}

			strcpy(current_allocated->name, buffername);
			current_allocated->address = i-3;

			current_free->start += buffersize + 8;
			current_free->size -= (buffersize + 8);
			printf("\nAllocated region name: %s\n", allocated_region->name);
			printf("Allocated region address: %d\n", allocated_region->address);
			printf("Free region start: %d\n", current_free->start);
			printf("Free region size: %d\n", current_free->size);
			break;
		}
	}

	heap_address = *casted;
	char alphabets[26];
	for (int u = 0; u < 26; u++) {
		alphabets[u] = (char)'a' + u;
	}

	printf("\nprinting buffer contents:\n");
	for (int x = heap_address + 8; x < buffersize + 8; x++) {
		memory[x] = alphabets[x%26];
		printf("%c", memory[x]);
	}
	printf("\n\n");
}

// deallocates the buffer on heap
void RemoveBuffer(){

}

// shows the state of the memory
void ShowMemory(all_frames * fs, char memory[], char occupied[]){
	printf("\nShowing Memory:\n");
	int i = 394;
	while (i > 255 && occupied[i] != '\0') {
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
		}
	}

	int j = 0;
	while (j < 255 && occupied[j] != '\0') {
		if (occupied[j] == '5') {
			printf("%c", memory[j]);
			j += 1;
		} else {
			j += 1;
		}
	}
}

int main () {
	int currentstacksize = 200;  
	int currentheapsize = 100;
	char memory[MEMSIZE];         // Buffer that will emulate stack and heap memory
	char occupied[MEMSIZE];

	for (int i = 0; i < MEMSIZE -1; i++) {
		memory[i] = '\0';
		occupied[i] = '\0';
	}

	all_frames fs;

	freelist * free_region;
	freelist freenode;
	free_region = &freenode;
	allocated * allocated_region; 
	allocated allocatednode;
	allocated_region = &allocatednode;  

	strcpy(allocated_region->name, "");
	allocated_region->address = -1;
	allocated_region->next = -1;
	free_region->start = 0;
	free_region->size = 100;
	free_region->next = NULL;

	SetNull(&fs);

	char name[] = "add";
	int address = rand() % 100;
	CreateFrame(&fs, &currentstacksize, name, address);

	char intname[] = "first";
	int intval = 5;
	CreateIntVar(&fs, memory, occupied, intname, intval);
	CreateIntVar(&fs, memory, occupied, intname, 10);
	CreateIntVar(&fs, memory, occupied, intname, 15);

	double idk = 453;
	double idk2 = 5987;
	CreateDoubleVar(&fs, memory, occupied, intname, idk);
	CreateDoubleVar(&fs, memory, occupied, intname, idk2);
	CreateCharVar(&fs, memory, occupied, intname, '9');

	CreateBuffer(&fs, memory, occupied, currentheapsize, allocated_region, free_region, "buffer", 4);

	all_frames * casted;
	casted = (all_frames*)&memory[395];
	*casted = fs;
	printf("idk what I am printing: %c \n", casted->frames[3].used);

	printf ("frame number: %d\n", fs.frames[0].number);
	printf ("function name: %s\n", fs.frames[0].name);
	printf ("function address: %d\n", fs.frames[0].functionaddress);
	printf ("frame address: %d\n", fs.frames[0].frameaddress);
	printf ("frame usage: %d\n", fs.frames[0].used);
	printf ("top of the stack: %d\n", top_of_stack);

	ShowMemory(&fs, memory, occupied);

	// printf ("size of free space on heap: %d\n", head->start);
	// printf ("starting address of free heap region: %d\n", head->size);
	return 0;
}
