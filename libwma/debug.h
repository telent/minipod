#ifdef DEBUG
#define DEBUGF(...) fprintf (stderr, __VA_ARGS__)
#define LOGF(...) fprintf (stderr, __VA_ARGS__)
#define LDEBUGF(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUGF(...) 
#define LDEBUGF(...) 
#define LOGF(...) 
#endif
