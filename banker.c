#include <stdio.h>
#include "campus.h"

int banker_command(const char *path) {
    FILE *f=fopen(path,"r"); if(!f) { perror(path); return 1; }
    int n,m,a[MAX_RES], alloc[MAX_JOBS][MAX_RES], maximum[MAX_JOBS][MAX_RES], need[MAX_JOBS][MAX_RES];
    if(fscanf(f,"%d%d",&n,&m)!=2 || n<1 || n>MAX_JOBS || m<1 || m>MAX_RES) goto invalid;
    for(int r=0;r<m;++r) if(fscanf(f,"%d",&a[r])!=1 || a[r]<0 || a[r]>10000) goto invalid;
    for(int i=0;i<n;++i) for(int r=0;r<m;++r)
        if(fscanf(f,"%d",&alloc[i][r])!=1 || alloc[i][r]<0 || alloc[i][r]>10000) goto invalid;
    for(int i=0;i<n;++i) for(int r=0;r<m;++r) {
        if(fscanf(f,"%d",&maximum[i][r])!=1 || maximum[i][r]<alloc[i][r] || maximum[i][r]>10000) goto invalid;
        need[i][r]=maximum[i][r]-alloc[i][r];
    }
    { char extra[2]; if(fscanf(f,"%1s",extra)==1) goto invalid; }
    if(fclose(f)!=0) { perror("closing resource file"); return 1; }
    printf("\n=== Banker's safety check: %s ===\nAvailable:",path);
    for (int r = 0; r < m; ++r) {
        printf(" %d", a[r]);
    }
    puts("\nNeed matrix:");
    for(int i=0;i<n;++i) { printf("P%d:",i); for(int r=0;r<m;++r) printf(" %d",need[i][r]); putchar('\n'); }
    int finished[MAX_JOBS]={0}, seq[MAX_JOBS], count=0;
    while(count<n) {
        int found=0;
        for(int i=0;i<n;++i) if(!finished[i]) {
            int can=1; for(int r=0;r<m;++r) if(need[i][r]>a[r]) can=0;
            if(can) { finished[i]=1; seq[count++]=i; found=1; for(int r=0;r<m;++r) a[r]+=alloc[i][r]; }
        }
        if(!found) break;
    }
    if(count==n) { printf("SAFE. Safe sequence: "); for(int i=0;i<n;++i) printf("%sP%d",i?" -> ":"",seq[i]); putchar('\n'); }
    else { printf("UNSAFE: no complete safe sequence exists. Unfinished:"); for(int i=0;i<n;++i) if(!finished[i]) printf(" P%d",i); putchar('\n'); }
    return 0;
invalid:
    fprintf(stderr,"Invalid resource file. Check dimensions, non-negative values and Allocation <= Maximum.\n"); fclose(f); return 1;
}
