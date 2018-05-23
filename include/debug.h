#ifndef _DEBUG_H_
#define _DEBUG_H_


extern __thread FILE *debug_output;

#ifdef VERBOSE
#define PRINT_DEBUG_INIT() do{                                   \
        char __filename[256];                                    \
        memset(__filename,0,256);                                \
        sprintf(__filename,"/tmp/debug_%d",__thread_id);         \
        debug_output=fopen(__filename,"w");                      \
    } while (0)
#else 
#define PRINT_DEBUG_INIT()
#endif
        
#ifdef VERBOSE
#define PRINT_DEBUG_FINALIZE() do{                          \
        fclose(debug_output);                               \
    } while (0)
#else
#define PRINT_DEBUG_FINALIZE()
#endif

#ifdef VERBOSE
#define PRINT_DEBUG(level,format, args...) do {                      \
        if(level<=VERBOSE){                                          \
            if(debug_output==NULL){                                  \
                debug_output=stdout;                                 \
            }                                                        \
            fprintf(debug_output,                                    \
                    format,                                          \
                    ## args);                                        \
            fflush(debug_output);                                    \
        }                                                            \
    } while (0)
#else
#define PRINT_DEBUG(level,format, args...)
#endif

#ifdef VERBOSE
#define PRINT_ERROR(format, args...) do {                            \
        if(debug_output==NULL){                                      \
            debug_output=stdout;                                     \
        }                                                            \
        fprintf(debug_output,                                        \
                format,                                              \
                ## args);                                            \
        fflush(debug_output);                                        \
        if(debug_output!=stdout){                                    \
            fprintf(stdout,                                          \
                    format,                                          \
                    ## args);                                        \
            fflush(stdout);                                          \
        }                                                            \
    } while (0)
#else
#define PRINT_ERROR(format, args...) do {                            \
        fprintf(stdout,                                              \
                format,                                              \
                ## args);                                            \
        fflush(stdout);                                              \
    } while (0)
#endif


#ifdef VERBOSE
extern __thread int __thread_id;
#define PRINT_DEBUG_THREAD(level,format, args...) do {                  \
        if(level<=VERBOSE){                                             \
            if(debug_output==NULL){                                     \
                debug_output=stdout;                                    \
            }                                                           \
            fprintf(debug_output,"Thread %d:", __thread_id);            \
            fprintf(debug_output,                                       \
                    format,                                             \
                    ## args);                                           \
            fflush(debug_output);                                       \
        }                                                               \
    } while (0)
#else
#define PRINT_DEBUG_THREAD(level,format, args...)
#endif



#ifdef VERBOSE
#define PRINT_BINARY(level,val_,length_) do {                        \
        if(level<=VERBOSE){                                          \
            if(debug_output==NULL){                                  \
                debug_output=stdout;                                 \
            }                                                        \
            fprintf(debug_output,"%x : ", (unsigned int)(val_));     \
            unsigned int iii= 1U << ((length_)-1);                   \
            while(iii >0){                                           \
                fprintf(debug_output,"%d",((val_) & iii)==iii);      \
                iii >>=1;                                            \
            }                                                        \
            fprintf(debug_output,"\n");                              \
            fflush(debug_output);                                    \
        }                                                            \
    } while (0)
#else
#define PRINT_BINARY(level,val_,length_)
#endif


#endif
