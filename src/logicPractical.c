/* Trabalho Prático, Lógica Computacional
 * Ano Letivo: 2020/2021
 * @author Bruno Monteiro, a43994, PL3
 * @author Duarte Arribas, a44585, PL3
 * @version 1.8
 */
//includes
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>
#include<inttypes.h>
//memory functions (documentation below)
#define MEMORY_ERROR -1
void *memalloc(size_t size);
void *memrealloc(void *ptr,size_t newSize);
void *memreallocp(void *ptr,size_t *newSize,size_t typeSize);
void clnmem(void *ptr);
//split functions (documentation below)
#define BUFFER_SIZE 20000
typedef enum{
  IN,
  OUT
}DELIMETER_STATE;
char **split(const char *string,const char *delimeter,size_t *splitArraySize);
void freeSplit(const size_t splitStringSize,char **splitString);
//search functions (documentation below)
int64_t clsearch(size_t size,char *array,char value);
int64_t clsearch2(size_t size,char *array,char value);
int64_t i64lsearch(size_t size,int64_t *array,int64_t value);
//horn
int horn(char *formula);
void removeRepeated(size_t andArraySize,char **andArray);
bool isSinglePositive(size_t andArraySize,char **andArray);
bool isPositive(char *andArray);
void transformImplication(size_t andArraySize,char **andArray);
bool findBottom(size_t andArraySize,char **andArray);

int main(void){
  //contains each formula
  size_t formulaSize=2;
  char *formula=memalloc(formulaSize*sizeof(char));
  formula[0]=' ';
  //saves the char that is being read
  int currentChar=0;
  //infinite loop that stops once all formulas have been read,
  //work on, and the output has been sent to the screen
  while(true){
    //read each formula
    while((currentChar=fgetc(stdin))!='\n'){
      //check if fgetc returned EOF; either an
      //error or the end-of-file has been reached
      if(currentChar==EOF){
        clnmem(formula);
        if(ferror(stdin)){
          return 1;
        }
        if(feof(stdin)){
          return 0;
        }
      }
      //save char and allocate more memory for the array
      formula[formulaSize-1]=currentChar;
      formula=memreallocp(formula,&formulaSize,sizeof(char));
    }
    //needed to interpret the formula as a string
    formula[formulaSize-1]=' ';
    formula=memreallocp(formula,&formulaSize,sizeof(char));
    formula[formulaSize-1]='\0';
    //get result from applying the horn algorithm and handle it accordingly
    int finalResult=horn(formula);
    if(finalResult<0){
      printf("NA\n");
    }
    else if(finalResult==0){
      printf("UNSAT\n");
    }
    else{
      printf("SAT\n");
    }
    //reset formula (will change on every iteration)
    clnmem(formula);
    formulaSize=1;
    formula=memalloc(formulaSize*sizeof(char));
  }
  //fallback
  clnmem(formula);
  return 0;
}

/**
 * Applies the horn algorithm, alongside some formatting on the given formula
 * @param *formula the given fnc formula
 * @return -1 if the horn algorithm is not applicable, 
 *          0 if the formula is contradictory
 *          1 if the formula is possible
 */
int horn(char *formula){
  //creates the split array of the formula by the `&` character
  size_t andArraySize=1;
  char **andArray=split(formula,"&",&andArraySize);
  //removes repeated positive literals
  removeRepeated(andArraySize,andArray);
  //checks if there are more than 1 positive literals in each clause
  //to check if the horn algorithm is not applicable
  if(!isSinglePositive(andArraySize,andArray)){
    freeSplit(andArraySize,andArray);
    return -1;
  }
  else{
    //transforms each clause in an implication
    transformImplication(andArraySize,andArray); 
    //applies the rest of the algorithm, and if bottom is found, 
    //conclude that the formula is contradictory; otherwise, conclude 
    //that the formula is possible
    if(findBottom(andArraySize,andArray)){
      freeSplit(andArraySize,andArray);
      return 0;
    }
    else{
      freeSplit(andArraySize,andArray);
      return 1;
    }
  }
}

/**
 * Removes repeated elements from all clauses of a formula
 * @param andArraySize the size of the split array
 * @param **andArray the given fnc formula split on the `&`
 */
void removeRepeated(size_t andArraySize,char **andArray){
  //buffer that holds the new clause, until it's finished
  size_t bufferSize=1;
  char *buffer=memalloc(bufferSize*sizeof(char));
  //iterate through the split array
  for(size_t i=0;i<andArraySize;i++){
    //iterate through each clause
    size_t arraySize=strlen(andArray[i]);
    for(size_t j=0;j<arraySize;j++){
      //if a proposition is found, check if it isn't in the buffer and is not negative
      //  if so, add it into the buffer; if not, remove the previous OR pipe and prevent the next one
      //if a proposition isn't found, just add the corresponding character
      if(isalpha(andArray[i][j])){
        int64_t letterPos=clsearch2(bufferSize-1,buffer,andArray[i][j]);
        if(letterPos==-1||andArray[i][j-1]=='~'){
          buffer[bufferSize-1]=andArray[i][j];
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));  
        }
        else{
          if(buffer[letterPos-1]=='~'){
            buffer[bufferSize-1]=andArray[i][j];
            buffer=memreallocp(buffer,&bufferSize,sizeof(char));    
          }
          else{
            bufferSize-=2; 
            buffer=memrealloc(buffer,bufferSize*sizeof(char));
          }
        }
      }
      else{
        buffer[bufferSize-1]=andArray[i][j];
        buffer=memreallocp(buffer,&bufferSize,sizeof(char));  
      }
    }
    //transform the buffer into a string and copy it into the clause
    buffer[bufferSize-1]='\0';
    strcpy(andArray[i],buffer);
    //reset the buffer
    clnmem(buffer);
    bufferSize=1;
    buffer=memalloc(bufferSize*sizeof(char));
  }
  //clean the buffer's memory, before exiting the function
  clnmem(buffer);
}

/**
 * Transforms each clause into an implication
 * @param andArraySize the size of the split array
 * @param **andArray the given fnc formula split on the `&`
 */
void transformImplication(size_t andArraySize,char **andArray){
  //buffer that holds the new clause, until it's finished
  size_t bufferSize=1;
  char *buffer=memalloc(bufferSize*sizeof(char));
  //iterate through the split array
  for(size_t i=0;i<andArraySize;i++){
    //if a single proposition `p` is found, change it to (TOP -> `p`)
    //  we represent TOP as a `+`
    //if more than one proposition is found, check if there is a positive proposition `p`
    //  if so, change it to (NegativePropositions -> `p`), where the negative propositions
    //    are separated by an &
    //  if not, change it to (NegativePropositions -> BOTTOM), where the negative propositions
    //    are separated by an &
    //    we represent BOTTOM as a `-`
    size_t arraySize=strlen(andArray[i]);
    if(arraySize==5){
      char newImp[10]=" (+ -> ";
      strncat(newImp,&andArray[i][2],1);
      strcat(newImp,")");
      strcpy(andArray[i],newImp);
    }
    else if(isPositive(andArray[i])){
      char pos=0;
      buffer[bufferSize-1]=' ';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]='(';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      for(size_t j=0;j<arraySize;j++){
        if(isalpha(andArray[i][j])&&andArray[i][j-1]=='~'){
          buffer[bufferSize-1]=andArray[i][j];
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]=' ';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]='&';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]=' ';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
        }
        else if(isalpha(andArray[i][j])){
          pos=andArray[i][j];
        }
      }
      buffer[bufferSize-3]='-';
      buffer[bufferSize-2]='>';
      buffer[bufferSize-1]=' ';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]=pos;
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]=')';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]='\0';
      strcpy(andArray[i],buffer);
      clnmem(buffer);
      bufferSize=1;
      buffer=memalloc(bufferSize*sizeof(char));
    }
    else{
      buffer[bufferSize-1]='(';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      //iterate through each clause
      for(size_t j=0;j<arraySize;j++){
        if(isalpha(andArray[i][j])&&andArray[i][j-1]=='~'){
          buffer[bufferSize-1]=andArray[i][j];
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]=' ';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]='&';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
          buffer[bufferSize-1]=' ';
          buffer=memreallocp(buffer,&bufferSize,sizeof(char));
        }
      }
      buffer[bufferSize-3]='-';
      buffer[bufferSize-2]='>';
      buffer[bufferSize-1]=' ';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]='-';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      buffer[bufferSize-1]=')';
      buffer=memreallocp(buffer,&bufferSize,sizeof(char));
      //transform the buffer into a string and copy it to each clause
      buffer[bufferSize-1]='\0';
      strcpy(andArray[i],buffer);
      //reset the buffer
      clnmem(buffer);
      bufferSize=1;
      buffer=memalloc(bufferSize*sizeof(char));
    }
  }
  //clean the buffer's memory, before exiting the function
  clnmem(buffer);
}

/**
 * Check if there's more than a single positive literal
 * @param andArraySize the size of the split array
 * @param **andArray the given fnc formula split on the `&`
 * @return true if there is only a single literal or none literals
 *  and false if there are multiple literals
 */
bool isSinglePositive(size_t andArraySize,char **andArray){
  
  //number of positive literals
  size_t positive=0;
  //iterate through the split array
  for(size_t i=0;i<andArraySize;i++){
    //reset the number of positive literals on every clause
    positive=0;
    //iterate through each clause
    size_t arraySize=strlen(andArray[i]);
    for(size_t j=0;j<arraySize;j++){

      //if a letter is found and not hasn't been reached, then a positive 
      //literal has been found, hence we increment the positive variable
      if(isalpha(andArray[i][j])&&andArray[i][j-1]!='~'){
        ++positive;
      }
    }
    //if there are more than 1 positive, return false
    if(positive>1){
      return false;
    }
  }
  //if there is less than 1 positive, return true
  return true;
}

/**
 * Check if there is a positive literal in the specified clause
 * @param *andArray the clause to check
 * @return true if there is a positive single literal and false otherwise
 */
bool isPositive(char *andArray){
  //check if not was reached
  bool reachedNot=false;
  //iterate through the clause
  size_t arraySize=strlen(andArray);
  for(size_t i=0;i<arraySize;i++){
    //reset the not variable on different OR or on the end of the clause
    if(andArray[i]=='|'||andArray[i]==')'){
      reachedNot=false;
    }
    //not reached
    if(andArray[i]=='~'){
      reachedNot=true;
    }
    //if a letter is found and not hasn't been reached, then a positive literal
    //has been found, hence we return true
    if(isalpha(andArray[i])){
      if(!reachedNot){
        return true;
      }
    }
  }
  //a positive literal hasn't been found, hence we return false
  return false;
}

/**
 * After applying the remainder of the horn algorithm,
 * by going through each clause, and adding the member implied
 * to the set, if all the the members that imply it are already in the set.
 * At the end, if the bottom is in the set, the formula is contradictory;
 * if not, it is possible
 * @param andArraySize the size of the split array
 * @param **andArray the given fnc formula split on the `&`
 * @return true if bottom is in the end set, and false otherwise
 */
bool findBottom(size_t andArraySize,char **andArray){
  bool propAnd=true;
  //the proposition set, that'll hold all `new` propositions, by
  //looking at the implications
  size_t propSetSize=1,propSetSizeInitial,alreadyEnteredSize=1;
  char *propositionSet=memalloc(propSetSize*sizeof(char));
  //check if the clause was already seen, to prevent it from adding unnecessary
  //propositions to the set
  int64_t *alreadyEntered=memalloc(alreadyEnteredSize*sizeof(int64_t));
  //add the initial TOP to the proposition set
  propositionSet[0]='+';
  propositionSet=memreallocp(propositionSet,&propSetSize,sizeof(char));
  //infinite loop; will exit when the size of the set after an iteration isn't changed
  //if it doesn't change, we conclude that no more implications can be entered
  while(true){
    //assign the size of the set to a variable; 
    //if they're different at the end, the loop may continue
    propSetSizeInitial=propSetSize;
    //iterate through the split array
    for(size_t i=0;i<andArraySize;i++){
      //controls whether all propositions that implicate the other are in the set
      propAnd=true;
      //iterate through each clause
      size_t arraySize=strlen(andArray[i]);
      for(size_t j=0;j<arraySize;j++){
        //break when the implication operator is reached
        if(andArray[i][j]=='>'){
          break;
        }
        //if the proposition is not in the set, break, because we can't conclude anything,
        //since, not all propositions are in the set
        if(isalpha(andArray[i][j])){
          if(clsearch(propSetSize-1,propositionSet,andArray[i][j])==-1){
            propAnd=false;
            break;
          }
        }
      }
      //if all propositions were found, add the implied one to the set, and the number of the clause
      //to the already entered clauses, to not enter them more than once
      if(propAnd&&i64lsearch(alreadyEnteredSize-1,alreadyEntered,i)==-1){
        alreadyEntered[alreadyEnteredSize-1]=i;
        alreadyEntered=memreallocp(alreadyEntered,&alreadyEnteredSize,sizeof(int64_t));
        propositionSet[propSetSize-1]=andArray[i][strlen(andArray[i])-2];
        propositionSet=memreallocp(propositionSet,&propSetSize,sizeof(char));
      }
    }
    //if the size of the set is equal to the temporary one, its size didn't
    //change, during the iteration, thus the function can end; if bottom was found,
    //return true; if not return false
    if(propSetSize==propSetSizeInitial){
      if(clsearch(propSetSize-1,propositionSet,'-')==-1){
        //cleanup memory
        clnmem(propositionSet);
        clnmem(alreadyEntered);
        return false;
      }
      else{
        //cleanup memory
        clnmem(propositionSet);
        clnmem(alreadyEntered);
        return true;
      }
    }
  }
}

//========================== Auxiliary Functions =========================//

//memory functions
/**
 * Handles malloc allocation.
 * If the memory cannot be allocated, it handles it properly by exiting the program.
 * @param size the initial size of the array (should account for different size objects)
 * @return the pointer to the new allocated memory
 */
void *memalloc(size_t size){
  void *temp=malloc(size);
  if(temp==NULL){
    exit(MEMORY_ERROR);
  }
  return temp;
}
/**
 * Handles realloc allocation.
 * If the memory cannot be reallocated, it handles it properly by exiting the program.
 * @param *ptr the pointer to the previous allocated memory
 * @param newSize the new size of the array (should account for different size objects)
 * @return the pointer to the new allocated memory
 */
void *memrealloc(void *ptr,size_t newSize){
  void *temp=realloc(ptr,newSize);
  if(temp==NULL){
    clnmem(ptr);
    exit(MEMORY_ERROR);
  }
  return temp;
}
/**
 * Handles realloc allocation.
 * If the memory cannot be reallocated, it handles it properly by exiting the program.
 * @param *ptr the pointer to the previous allocated memory
 * @param *size size of the previous array to increase by 1
 * @param typeSize the size of the type
 * @return the pointer to the new allocated memory
 */
void *memreallocp(void *ptr,size_t *size,size_t typeSize){
  ++(*size);
  void *temp=realloc(ptr,*size*typeSize);
  if(temp==NULL){
    clnmem(ptr);
    exit(MEMORY_ERROR);
  }
  return temp;
}
/**
 * Frees and clears memory spaces
 * @param *ptr the pointer the allocated memory
 */
void clnmem(void *ptr){
  free(ptr);
  ptr=NULL;
}

//split functions
/**
 * Splits the given string into an array of strings by the specified delimeter
 * Requires a call to freeSplit in the end
 * Example: split("a1:b1:c1:d1:e1:f1:g1",":",&size)->[a1,b1,c1,d1,e1,f1,g1]
 * @param *string the string to be split
 * @param *delimeter the string to server as delimeter
 * @param *splitArraySize the initial size of the split array
 * @return the split array
 */
char **split(const char *string,const char *delimeter,size_t *splitArraySize){
  char **splitArray=memalloc(*splitArraySize*sizeof(char*));
  for(size_t i=0;i<*splitArraySize;i++){
    splitArray[i]=memalloc(BUFFER_SIZE*sizeof(char));
  }
  DELIMETER_STATE currState=OUT;
  size_t strMax=0,i=0;
  for(;i<strlen(string);i++){
    if(currState==OUT){
      if(string[i]==delimeter[0]){
        currState=IN;
      }  
    }
    if(currState==IN){
      bool belongsToString=true;
      size_t j,k;
      for(j=i,k=0;k<strlen(delimeter)&&j<strlen(string);j++,k++){
        if(string[j]!=delimeter[k]){
          belongsToString=false;
        }
      }
      if(!belongsToString){
        i+=strlen(delimeter)-2;
      }
      else{
        size_t m,n=0;
        char temp[BUFFER_SIZE];
        for(m=strMax;m<i;m++,n++){
          temp[n]=string[m];
        }
        temp[n]='\0';
        strcpy(splitArray[(*splitArraySize)-1],temp);
        ++(*splitArraySize);
        splitArray=realloc(splitArray,*splitArraySize*sizeof(char*));
        splitArray[*splitArraySize-1]=memalloc(BUFFER_SIZE*sizeof(char));
        strMax=m+strlen(delimeter);
        i+=strlen(delimeter)-1;
      }
      currState=OUT;
    }
  }
  if(i==strlen(string)){
    size_t m,n=0;
    char temp[BUFFER_SIZE];
    for(m=strMax;m<i;m++,n++){
      temp[n]=string[m];
    }
    temp[n]='\0';
    strcpy(splitArray[(*splitArraySize)-1],temp);
  }
  return splitArray;
}
/**
 * Frees the split string array
 * @param splitStringSize the size of the split string array
 * @param **splitString the splitString array
 */
void freeSplit(const size_t splitStringSize,char **splitString){
  for(size_t i=0;i<splitStringSize;i++){
    clnmem(splitString[i]);
  }
  clnmem(splitString);
}

//search functions
/**
 * Searches the array for the given value. (char version)
 * @param size the size of the array
 * @param *array the array to search
 * @param value the value to search for
 * @return the first index of value in the array or -1 if it doesn't exit
 */
int64_t clsearch(size_t size,char *array,char value){
  for(size_t i=0;i<size;i++){
    if(array[i]==value){
      return i;
    }
  }
  return -1;
}

int64_t clsearch2(size_t size,char *array,char value){
  for(size_t i=0;i<size;i++){
    if(array[i]==value){
      if(array[i-1]!='~'){
        return i;
      }
    }
  }
  return -1;
}
/**
 * Searches the array for the given value. (int64_t version)
 * @param size the size of the array
 * @param *array the array to search
 * @param value the value to search for
 * @return the first index of value in the array or -1 if it doesn't exit
 */
int64_t i64lsearch(size_t size,int64_t *array,int64_t value){
  for(size_t i=0;i<size;i++){
    if(array[i]==value){
      return i;
    }
  }
  return -1;
}