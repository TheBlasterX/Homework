#include<mpi.h>
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAG_START 1
#define SOBEL_TAG 2
#define MEAN_REMOVAL_TAG 3
#define TRANSMISSION_TAG 4
#define END_TAG 5
#define END_ROOT_TAG 6
#define MAX_NODE 100

int *isplit(const char *s, int *size, const char *delim);
int M,N;


int main(int argc, char * argv[]) {
    int rank;
    int nProcesses;
    int i,tag;
    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Request request;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    int processed_lines=0;
    // citirea din topologie
    FILE *file = fopen(argv[1], "r");
    size_t size =100;
    char *aux;
    aux = malloc(100);
    for(i = 0;i < rank ; i++){
        getline(&aux, &size , file);
    }
    
    int nod;
    fscanf(file,"%d",&nod);
    if(nod != rank){
        printf("erare la citire\n");
        return 0;
    }
    int number_neighborhood;
    char aux_char;
    fscanf(file,"%c",&aux_char);
    getline(&aux, &size , file);
    int *neighborhood = isplit(aux, &number_neighborhood, "  \n");
    int n = rank,j;
    int width;
    int num_line ;
    
    if(rank == 0){
        //citirea imaginelor
        FILE *file_img = fopen(argv[2], "r");
        int number_image;
        fscanf(file_img,"%d",&number_image);
        for(int k = 0 ; k < number_image; k++){
            char *filtru, *name_image , *name_image_out;
            filtru = malloc(50);
            name_image = malloc(100);
            name_image_out = malloc(100);
            fscanf(file_img,"%s %s %s",filtru, name_image , name_image_out);
            
            if(strcmp(filtru,"sobel") == 0){
                tag = SOBEL_TAG;
            }
            else if(strcmp(filtru,"mean_removal") == 0){
                tag = MEAN_REMOVAL_TAG;
            }
            char *magic, *creator_info;
            magic = malloc(2);
            creator_info = malloc(100);
            int img_intensity;
            FILE *file;
            size_t size =0;
            file = fopen(name_image, "r");
            if(!file)
            {
                printf("Eroare: nu a fost  gasit fisierul\n");
                fclose(file);
            }
            size = 2;
            getline(&magic, &size , file);
            size =100;
            getline(&creator_info, &size , file);
            
            fscanf(file,"%d",&M);
            fscanf(file,"%d",&N);
            fscanf(file,"%d",&img_intensity);
            int **img;
            img = (int**)malloc((N+2)*sizeof(int*));
            for(int i=0; i<N+2; i++){
                img[i] = (int*)malloc((M+1)*sizeof(int));
            }
            for(int i=0; i<N+2; i++){
                for(int j=0; j<M+2; j++){
                    if(i == 0 || i == N + 1 || j == 0 || j == M+1){
                        img[i][j] = 0;
                    }
                    else{
                        fscanf(file,"%d",&img[i][j]);
                    }
                }
            }
            fclose(file);
            n = N;
            width = M + 2;
            for(i = 0;i<number_neighborhood; i++){

                if(neighborhood[i] != rank){
                    // daca va fi ultimul copil va analiza liniile ramase
                    if(i ==  number_neighborhood-1){
                        num_line = n / (number_neighborhood)+ n % (number_neighborhood);
                    }
                    else{
                        num_line = n / (number_neighborhood);
                    }
                    
                    // transmit numarul de linii si de coloane
                    num_line +=2;
                    MPI_Send(0,0,MPI_INT,neighborhood[i] , tag , MPI_COMM_WORLD);
                    MPI_Send(&num_line, 1, MPI_INT, neighborhood[i] , TAG_START , MPI_COMM_WORLD);
                    MPI_Send(&width, 1, MPI_INT, neighborhood[i] , TAG_START , MPI_COMM_WORLD);
                    num_line -=2;

                    // transmit liniile
                    if(i ==  number_neighborhood-1){
                        for(j = (n / (number_neighborhood) * (i)) ; j <= n + 1; j++){
                            MPI_Send(img[j] , width , MPI_INT, neighborhood[i] , TRANSMISSION_TAG , MPI_COMM_WORLD);
                        }
                    }
                    else{
                        for(j = (num_line * (i)) ; j <= (num_line * (i+1)) +1 ; j++){
                            MPI_Send(img[j] , width , MPI_INT, neighborhood[i] , TRANSMISSION_TAG , MPI_COMM_WORLD);
                        }
                    }
                }
            }
            for(int i=0; i<N+2; i++){
                free(img[i]);
            }
            free(img);
            
            int line;
            int **con_img;
            con_img = (int**)malloc((n)*sizeof(int*));
            for(int i=0; i<n; i++){
                con_img[i] = (int*)malloc((width)*sizeof(int));
            }
            int kk=0;
            for(i = 0;i < number_neighborhood ; i++){
                // primesc liniile in ordine
                if(neighborhood[i] != rank){
                    MPI_Recv(&line, 1, MPI_INT, neighborhood[i]  , TRANSMISSION_TAG , MPI_COMM_WORLD, &status);
                    for(int j=0;j < line ;j++){
                        MPI_Recv(con_img[kk], width , MPI_INT, neighborhood[i] , TRANSMISSION_TAG, MPI_COMM_WORLD, &status);
                        kk++;
                    }
                }
            }
            // scriu in fisier
            FILE* file2 = fopen(name_image_out, "w");
            fprintf(file2, "%s%s",magic,creator_info);
            fprintf(file2, "%d %d\n%d\n",M,N,img_intensity );
            for(int i=0; i<N; i++){ 
                for(int j=1; j<M+1; j++){
                    fprintf(file2, "%d\n",con_img[i][j] );
                }
            }
            for(int i=0; i<n; i++){
                free(con_img[i]);
            }
            free(con_img);
            free(magic);
            free(creator_info);
            fclose(file2);
        }
        fclose(file_img);
        fclose(file);
        for(i = 0;i < number_neighborhood ; i++){
            MPI_Send(0,0,MPI_INT,neighborhood[i] , END_TAG , MPI_COMM_WORLD);
        }
        int end=0;
        int statistic[MAX_NODE];
        statistic[0]=0;
        for(int i=1;i<MAX_NODE;i++){
            statistic[i]=-1;
        }
        //primesc statistica
        while(end!= number_neighborhood){
            int n[2];
            MPI_Recv(n, 2 , MPI_INT, MPI_ANY_SOURCE  ,MPI_ANY_TAG , MPI_COMM_WORLD, &status);
            if(status.MPI_TAG == END_ROOT_TAG){
                end++;
            }
            else{
                statistic[n[0]]=n[1];
            }
            
        }
        
        FILE* file_out = fopen(argv[3], "w");
        for(i=0;i<MAX_NODE;i++){
            if(statistic[i] != -1 ){
                fprintf(file_out,"%d: %d\n",i,statistic[i]);
            }
        }
        fclose(file_out);
        
    }
    else{
        
        while(1){
            
            int children[number_neighborhood-1],father;
            int p=0;
            
            MPI_Recv(0, 0, MPI_INT, MPI_ANY_SOURCE ,MPI_ANY_TAG , MPI_COMM_WORLD, &status);
            // aflu are sunt copii si care este parintele
            for(i = 0;i < number_neighborhood; i++){
                if(neighborhood[i] != status.MPI_SOURCE){
                    children[i-p] = neighborhood[i];
                }
                else{
                    p++;
                    father = neighborhood[i];
                }
            }
            
            // daca primesc tagul de terminare cer de la copii statistica
            if(status.MPI_TAG == END_TAG){
                for(i = 0;i < number_neighborhood -1 ; i++){
                    MPI_Send(0,0,MPI_INT,children[i] , END_TAG , MPI_COMM_WORLD);
                }
                int end=0;
                while(end!= number_neighborhood-1){
                    int n[2];
                    MPI_Recv(n, 2 , MPI_INT,MPI_ANY_SOURCE  ,MPI_ANY_TAG , MPI_COMM_WORLD, &status);
                    if(status.MPI_TAG == END_ROOT_TAG){
                        end++;
                    }else{
                        MPI_Send(n,2,MPI_INT,father,END_TAG, MPI_COMM_WORLD);
                    }
                }
                // trasmit statistica la parinte
                int n[2]={rank,processed_lines};
                MPI_Send(n,2,MPI_INT,father,END_TAG, MPI_COMM_WORLD);
                MPI_Send(0,0,MPI_INT,father,END_ROOT_TAG, MPI_COMM_WORLD);
                break;
            }
            else{
                // primesc liniile
                tag = status.MPI_TAG ;
                MPI_Recv(&n, 1, MPI_INT, MPI_ANY_SOURCE , TAG_START, MPI_COMM_WORLD, &status);
                MPI_Recv(&width, 1, MPI_INT, MPI_ANY_SOURCE , TAG_START, MPI_COMM_WORLD, &status);
                int **img_matrix;
                img_matrix = malloc((n)*sizeof(int*));
                if(img_matrix == NULL){
                }
                for(int i=0; i<n; i++){
                    img_matrix[i] = malloc((width)*sizeof(int));
                    if(img_matrix[i] == NULL){
                    }
                }
                
                for(i = 0 ;i < n ; i++){
                    MPI_Recv(img_matrix[i], width , MPI_INT, MPI_ANY_SOURCE , TRANSMISSION_TAG, MPI_COMM_WORLD, &status);
                    
                }
                n=n-2;
                //transmit liniile
                for(i = 0;i < number_neighborhood -1 ; i++){
                    
                    if(i ==  number_neighborhood-2){
                        num_line = n / (number_neighborhood-1)+ n % (number_neighborhood-1);
                    }
                    else{
                        num_line = n / (number_neighborhood-1);
                    }
                    num_line+=2;
                    MPI_Send(0,0,MPI_INT,children[i] , tag , MPI_COMM_WORLD);
                    MPI_Send(&num_line, 1, MPI_INT, children[i] , TAG_START , MPI_COMM_WORLD);
                    MPI_Send(&width,    1, MPI_INT, children[i] , TAG_START , MPI_COMM_WORLD);
                    
                    num_line-=2;
                    if(i ==  number_neighborhood-2){
                        int k = 0;
                        for(j = (n / (number_neighborhood-1) * (i)) ; j <= n + 1; j++){
                            k++;
                            MPI_Send(img_matrix[j] , width , MPI_INT, children[i] , TRANSMISSION_TAG , MPI_COMM_WORLD);
                            
                        }
                    }
                    else{
                        for(j = (num_line * (i)) ; j <= (num_line * (i+1)) +1 ; j++){
                            
                            MPI_Send(img_matrix[j] , width , MPI_INT, children[i] , TRANSMISSION_TAG , MPI_COMM_WORLD);
                        }
                    }
                    
                }
                if(number_neighborhood > 1){
                    //primesc imaginea de la copii
                    int line=0;
                    int **con_img;
                    con_img = malloc((n+1)*sizeof(int*));
                    
                    for(int i=0; i<n+1; i++){
                        con_img[i] = malloc((width)*sizeof(int));
                    }
                    
                    int k=0;
                    for(i = 0;i < number_neighborhood -1 ; i++){
                        
                        MPI_Recv(&line, 1, MPI_INT, children[i]  , TRANSMISSION_TAG , MPI_COMM_WORLD, &status);
                        for(int j=0;j < line ;j++){
                            MPI_Recv(con_img[k], width , MPI_INT, children[i] , TRANSMISSION_TAG, MPI_COMM_WORLD, &status);
                            k++;
                        }
                    }
                    //transmit imaginea la parinte
                    MPI_Send(&k , 1 , MPI_INT, father , TRANSMISSION_TAG , MPI_COMM_WORLD);
                    for(int i=0; i < k; i++){
                        MPI_Send(con_img[i] , width , MPI_INT, father , TRANSMISSION_TAG , MPI_COMM_WORLD);
                    }
                    for(int i=0; i<n+1; i++){
                        free(con_img[i]);
                    }
                    free(con_img);
                    
                    
                }
                else{ 
                    int matrix_convolution[3][3];
                    if(tag == SOBEL_TAG){
                        matrix_convolution[0][0]=1;
                        matrix_convolution[0][1]=0;
                        matrix_convolution[0][2]=-1;
                        
                        matrix_convolution[1][0]=2;
                        matrix_convolution[1][1]=0;
                        matrix_convolution[1][2]=-2;
                        
                        matrix_convolution[2][0]=1;
                        matrix_convolution[2][1]=0;
                        matrix_convolution[2][2]=-1;
                        
                    }
                    else{
                        for(i = 0 ; i < 3 ;i++){
                            for(j = 0 ; j < 3 ;j++){
                                matrix_convolution[i][j] = -1;
                            }
                        }
                        matrix_convolution[1][1] = 9;
                    }
                    
                    int **con_img;
                    con_img = malloc((n+1)*sizeof(int*));
                    
                    for(int i=0; i<n+1; i++){
                        con_img[i] = malloc((width)*sizeof(int));
                    }
                    
                    
                    for(int i=1; i<(n+1); i++){
                        for(int j=1; j<(width-1); j++){
                            int sum=0;
                            if(tag == SOBEL_TAG){
                                sum = 127;
                            }
                            if(j+1 != width && i+1 < width ){
                                for(int k=(i-1); k<=(i+1); k++){
                                    for(int l=(j-1); l<=(j+1); l++){
                                        sum = sum + img_matrix[k][l] * matrix_convolution[abs(i -k -1)][abs(j-l-1)];
                                    }
                                }
                                if(sum < 0 ){
                                    con_img[i][j]=0;
                                }
                                else if (sum > 255){
                                    con_img[i][j]=255;
                                }
                                else{
                                    con_img[i][j]=sum;
                                }
                            }
                        }
                        processed_lines++;
                    }
                    //transmit imaginea la parinte   
                    int num = n;
                    MPI_Send(&num , 1 , MPI_INT, father , TRANSMISSION_TAG , MPI_COMM_WORLD);
                    for(int i=1; i <= num; i++){
                        MPI_Send(con_img[i] , width , MPI_INT, father , TRANSMISSION_TAG , MPI_COMM_WORLD);
                    }
                    for(int i=0; i<n+1; i++){
                        free(con_img[i]);
                    }
                    free(con_img);
                    
                    
                }
                for(int i=0; i<n; i++){
                    free(img_matrix[i]);
                }
                free(img_matrix);
            }
        }
        
    }
    MPI_Finalize();
    return 0;
}

char *copy_string(const char *s)
{
    char *copy = malloc(strlen(s) + 1);
    
    if (copy != NULL)
        strcpy(copy, s);
    
    return copy;
}
int *isplit(const char *s, int *size, const char *delim)
{
    char *copy = copy_string(s);
    int *result = NULL;
    
    *size = 0;
    
    for (char *tok = strtok(copy, delim);
         tok != NULL;
         tok = strtok(NULL, delim)){
        
        (*size)++;
        result = realloc(result, *size * sizeof *result);
        result[*size - 1] = atoi(tok);
    }
    free(copy);
    
    return result;
}

