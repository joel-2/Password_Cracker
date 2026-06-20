#include <stdio.h> // for file I/O and printf
#include <string.h> // for strlen, strcmp, and strcspn
#include <openssl/evp.h> // for EVP digest functions
# include <pthread.h> // for multithreading 
#include "rules.h" // for rule function declarations

// shared state passed to every thread
typedef struct {
    FILE *wordlist;            // shared file handle
    char target_hash[129];     // hash we're trying to crack
    const EVP_MD *md;          // which algorithm
    volatile int found;        // stop flag - 1 means someone cracked it
    pthread_mutex_t mutex;     // protects file reading and output
    long attempts;             // total attempts across all threads
    int use_rules;            // whether to apply rules or not
} CrackJob;

void bytes_to_hex(unsigned char *bytes, int len, char *out) { //converts byte array to hex string
    for (int i = 0; i < len; i++) { 
        sprintf(out + (i * 2), "%02x", bytes[i]); // convert each byte to 2 hex chars
    }
    out[len * 2] = '\0'; // null-terminate the string
}

int crack(const char *word, const char *target_hash, const EVP_MD *md) { //function to check does this word matches the target hash
    unsigned char digest[EVP_MAX_MD_SIZE]; // buffer to hold the computed hash
    unsigned int digest_len; // evp tells us how long the hash is, but for MD5 it will always be 16 bytes
    char hex[129]; //buffer big enough to hold SHA512 hash in hex (128 chars + null terminator)

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();    // create a new digest context
    EVP_DigestInit(ctx, md);         // initialize the context with the selected hashing algorithm
    EVP_DigestUpdate(ctx, word, strlen(word)); // feed the word into the hash function
    EVP_DigestFinal(ctx, digest, &digest_len); // get result and length of the hash
    EVP_MD_CTX_free(ctx);                   // clean up the context

    bytes_to_hex(digest, digest_len, hex); // convert the binary hash to a hex string
    return strcmp(hex, target_hash) == 0; // compare the computed hash with the target hash and return 1 if they match, 0 otherwise
}

void *worker(void *arg) { // thread function to read lines from the wordlist and check against the target hash
    CrackJob *job = (CrackJob *)arg;  // cast the void pointer back to CrackJob
    char line[256];

    while (1) {
        // check stop flag before doing any work
        pthread_mutex_lock(&job->mutex);
        if (job->found) { pthread_mutex_unlock(&job->mutex); break; }  // bail early
        int got_line = (fgets(line, sizeof(line), job->wordlist) != NULL);
        job->attempts++;
        pthread_mutex_unlock(&job->mutex);

        if (!got_line) break;

        line[strcspn(line, "\r\n")] = '\0';
       

        // try the word as-is
        if (crack(line, job->target_hash, job->md)) {
            pthread_mutex_lock(&job->mutex);
            if (!job->found) {
                printf("[+] CRACKED after %ld attempts: %s\n", job->attempts, line);
                job->found = 1;
            }
            pthread_mutex_unlock(&job->mutex);
            break;
        }

        // try every rule mutation if --rules flag is set
        if (job->use_rules) {
            char mutated[256];
            for (int i = 0; i < rule_count; i++) {
                rules[i](line, mutated);                          // apply rule to get mutated word
                
                if (crack(mutated, job->target_hash, job->md)) {
                    pthread_mutex_lock(&job->mutex);
                    if (!job->found) {
                        printf("[+] CRACKED after %ld attempts: %s (rule: %d)\n", job->attempts, mutated, i);
                        job->found = 1;
                    }
                    pthread_mutex_unlock(&job->mutex);
                    goto done;  // break out of both the for loop and while loop
                }
            }
        }
        continue;
        done:
        break;
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <wordlist> <hash> <algo> <threads> [--rules]\n", argv[0]);
        return 1;
    }

    char *wordlist_path = argv[1];
    char *target_hash   = argv[2];
    char *algo          = argv[3];
    int   num_threads   = atoi(argv[4]);  // atoi converts string to int

    const EVP_MD *md = NULL;
    if      (strcmp(algo, "md5")    == 0) md = EVP_md5();
    else if (strcmp(algo, "sha1")   == 0) md = EVP_sha1();
    else if (strcmp(algo, "sha256") == 0) md = EVP_sha256();
    else if (strcmp(algo, "sha512") == 0) md = EVP_sha512();
    else { printf("Unknown algorithm: %s\n", algo); return 1; }

    FILE *f = fopen(wordlist_path, "r");
    if (!f) { perror("Failed to open wordlist"); return 1; }

    // set up the shared job
    CrackJob job;
    job.wordlist = f;
    strncpy(job.target_hash, target_hash, sizeof(job.target_hash));
    job.md       = md;
    job.found    = 0;
    job.attempts = 0;
    // check if --rules flag was passed
    job.use_rules = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rules") == 0) job.use_rules = 1;
    }

    pthread_mutex_init(&job.mutex, NULL);  // initialise the mutex

    // spawn threads
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker, &job);
    }

    // wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    if (!job.found) {
        printf("[-] Not found after %ld attempts\n", job.attempts);
    }

    pthread_mutex_destroy(&job.mutex);  // clean up mutex
    fclose(f);
    return 0;
}