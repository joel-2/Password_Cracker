#include <stdio.h> // for file I/O and printf
#include <string.h> // for strlen, strcmp, and strcspn
#include <openssl/md5.h> // for MD5 functions

void bytes_to_hex(unsigned char *bytes, int len, char *out) { //converts byte array to hex string
    for (int i = 0; i < len; i++) { 
        sprintf(out + (i * 2), "%02x", bytes[i]); // convert each byte to 2 hex chars
    }
    out[len * 2] = '\0'; // null-terminate the string
}

int md5_crack(const char *word, const char *target_hash) { //fuction to check does this word matches the target hash
    unsigned char digest[MD5_DIGEST_LENGTH]; // buffer to hold the MD5 hash (16 bytes)
    char hex[33]; // buffer to hold the hex string (32 chars + null terminator)

    MD5((unsigned char *)word, strlen(word), digest); // compute the MD5 hash of the input word
    bytes_to_hex(digest, MD5_DIGEST_LENGTH, hex); // convert the hash to a hex string

    return strcmp(hex, target_hash) == 0; // compare the computed hash with the target hash, return 1 if they match, 0 otherwise
}

int main(int argc, char *argv[]) { //main function to read the wordlist and check each word against the target hash
    if (argc != 3) { // check if the correct number of arguments is provided
        printf("Usage: %s <wordlist> <md5hash>\n", argv[0]); // print usage message if arguments are missing
        return 1;
    }

    char *wordlist_path = argv[1]; // path to the wordlist file
    char *target_hash   = argv[2]; // target MD5 hash to crack

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

        if (md5_crack(line, target_hash)) { // check if the current line's hash matches the target hash
            printf("[+] CRACKED after %ld attempts: %s\n", attempts, line);
            fclose(f);
            return 0;
        }
    }

    printf("[-] Not found after %ld attempts\n", attempts); // print message if the target hash was not found in the wordlist
    fclose(f);
    return 1;
}