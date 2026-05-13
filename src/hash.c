#include <stdio.h> // for file I/O and printf
#include <string.h> // for strlen, strcmp, and strcspn
#include <openssl/evp.h> // for EVP digest functions

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


int main(int argc, char *argv[]) { //main function to read the wordlist and check each word against the target hash
    if (argc != 4) { // check if the correct number of arguments is provided
        printf("Usage: %s <wordlist> <hash> <algo>\n", argv[0]); // print usage message if arguments are missing
        printf("Supported algorithms: md5, sha1, sha256, sha512\n"); // print supported algorithms
        return 1;
    }

    char *wordlist_path = argv[1]; // path to the wordlist file
    char *target_hash   = argv[2]; // target MD5 hash to crack
    char *algo          = argv[3]; // hashing algorithm to use implemented in the md5_crack function (currently only MD5, but we set up for future extension)

    const EVP_MD *md = NULL; // pointer to the selected hashing algorithm
    if      (strcmp(algo, "md5")    == 0) md = EVP_md5();  // select the appropriate hashing algorithm based on user input
    else if (strcmp(algo, "sha1")   == 0) md = EVP_sha1(); // SHA1 is not implemented in the md5_crack function, but we set it here for future extension
    else if (strcmp(algo, "sha256") == 0) md = EVP_sha256(); // SHA256 is not implemented in the md5_crack function, but we set it here for future extension
    else if (strcmp(algo, "sha512") == 0) md = EVP_sha512(); // SHA512 is not implemented in the md5_crack function, but we set it here for future extension
    else {
        printf("Unknown algorithm: %s\n", algo);
        return 1;
    }

    FILE *f = fopen(wordlist_path, "r"); // open the wordlist file for reading
    if (!f) {
        perror("Failed to open wordlist"); // print error message if file cannot be opened
        return 1;
    }

    char line[256]; // buffer to hold each line from the wordlist
    long attempts = 0;

    while (fgets(line, sizeof(line), f)) { // read each line from the wordlist
        line[strcspn(line, "\r\n")] = '\0';  // strip newline
        attempts++;

        if (crack(line, target_hash, md)) { // check if the current line's hash matches the target hash
            printf("[+] CRACKED after %ld attempts: %s\n", attempts, line);
            fclose(f);
            return 0;
        }
    }

    printf("[-] Not found after %ld attempts\n", attempts); // print message if the target hash was not found in the wordlist
    fclose(f);
    return 1;
}