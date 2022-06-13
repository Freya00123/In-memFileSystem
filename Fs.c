// Implementation of the File System ADT
// Written by: Heqing Huang

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileType.h"
#include "Fs.h"

struct FsNode {
    char *name;
    struct FsNode *h_prev;
    struct FsNode *l_next;
    struct FsNode *prev;
    struct FsNode *next;
    FileType type;
};

typedef struct FsNode *Node;

struct FsRep {
    Node root;
    Node curr_dir;
};

// helper function declaration
Node NewNode(char name[], FileType type);
void NodeFree(Node node);
Node lookInDir (Node node, char* name);
Node create_here (Node node, Node prev, Node h_prev, char *name, FileType type);
void print_current_dir(Node curr);
void tree(Node n, int level);


Fs FsNew(void) {
    Fs fs = malloc(sizeof(struct FsRep));
    fs->root = NewNode("simple_root", DIRECTORY);
    fs->curr_dir = fs->root;
    return fs;
}

void FsGetCwd(Fs fs, char cwd[PATH_MAX + 1]) {
    strcpy(cwd, "");
    Node curr = fs->curr_dir;
    char path[PATH_MAX + 1];
    while(curr != NULL) {
        if(curr->h_prev != NULL) { 
            // not the root dir
            strcpy(path, cwd);
            strcpy(cwd, "/");
            strcat(cwd, curr->name);
            strcat(cwd, path);
        }
        curr = curr->h_prev;
    }
    if (strcmp(cwd, "") == 0) {
        strcpy(cwd, "/");
    }
}

void FsFree(Fs fs) {
    NodeFree(fs->root);
    free(fs);
}

void FsMkdir(Fs fs, char *path) {
    char *path_string = strdup(path);
    Node curr = fs->curr_dir;
    // seperate the path by delimiter "/"
    // and loop each name 
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }
        if (strcmp(curr_token,".")==0) {
            // if the curr name is "." 
            if (next_token == NULL) {
                // if the next token is null
                // exit(file exists)
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkdir: cannot create directory \'%s\': File exists\n", path);
                return;
            }
        } else if (strcmp(curr_token,"..")==0) {
            // if the curr name is ".."
            if (next_token == NULL) {
                // if the next token is null
                // exit(file exists)
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkdir: cannot create directory \'%s\': File exists\n", path);
                return;
            } else if (curr->h_prev != NULL) {
                // if the next token is not null and the h_prev is not null
                // back to the h_prev
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkfile: cannot create directory \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            // create find_node to find the node with curr_token
            Node find_node = lookInDir(curr->l_next, curr_token);
            if (find_node == NULL) { 
                //dir not exist
                if (next_token == NULL) { 
                    //this is the one we are creating.
                    curr->l_next = create_here(curr->l_next, NULL, curr, curr_token, DIRECTORY);
                } else { 
                    // this is a prefix folder but not exist
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkdir: cannot create directory \'%s\': No Such file or directory\n", path);
                    return;
                }
            } else {
                // dir exists
                if (next_token == NULL) {
                    // next_token is null, find a duplicated name 
                    // exit(file exists)
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkdir: cannot create directory \'%s\': File exists\n", path);
                    return;
                } else if (find_node->type == REGULAR_FILE) {
                    // find_node is a regular file 
                    // exit(not a dir)
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkdir: cannot create directory \'%s\': Not a directory\n", path);
                    return;
                } else {
                    // enter the find_node
                    curr = find_node;
                }
            }
        }
    }
    free (curr_token);
    free (next_token);
    free (path_string);
}

void FsMkfile(Fs fs, char *path) {
    char *path_string = strdup(path);
    Node curr = fs->curr_dir;
    // 
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }
        if (strcmp(curr_token, ".") == 0) {
            // if the name is "."
            if (next_token == NULL) {
                // if the next name is null
                // exit (file exists)
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkfile: cannot create directory \'%s\': File exists\n", path);
                return;
            }
        } else if (strcmp(curr_token, "..") == 0) {
            // if the name is ".."
            if (next_token == NULL) {
                // if the next name is null
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkfile: cannot create directory \'%s\': File exists\n", path);
                return;
            } else if (curr->h_prev != NULL) {
                // if the h_prev is not null
                // back to h_prev
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkfile: cannot create directory \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            // create find_node to find the node with curr name
            Node find_node = lookInDir(curr->l_next, curr_token);
            if (find_node == NULL) { 
                //dir not exist
                if (next_token == NULL) { 
                    //this is the one we are creating.
                    curr->l_next = create_here(curr->l_next, NULL, curr, curr_token, REGULAR_FILE);
                }
                else { 
                    // this is a prefix folder but not exist
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkfile: cannot create directory \'%s\': No Such file or directory\n", path);
                    return;
                }
            } else {
                // dir exists
                if (next_token == NULL) {
                    // next token is null
                    // exit(file exists)
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkfile: cannot create directory \'%s\': File exists\n", path);
                    return;
                } else if (find_node->type == REGULAR_FILE) {
                    // find_node is a file
                    // exit(not a dir)
                    free (curr_token);
                    free (next_token);
                    free (path_string);
                    printf("mkfile: cannot create directory \'%s\': Not a directory\n", path);
                    return;
                } else {
                    // enter the find_node
                    curr = find_node;
                }
            }
        }    
    }
    free (curr_token);
    free (next_token);
    free (path_string);
}

void FsCd(Fs fs, char *path) {
    // if the path is NULL
    // back to the root    
    if (path == NULL) {
        fs->curr_dir = fs->root;
        return;
    }
    char *path_string = strdup(path);
    Node curr = fs->curr_dir;
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }
        
        if (strcmp(curr_token, ".") == 0) {
            // if the curr name is "."
            curr = fs->curr_dir;
        } else if (strcmp(curr_token, "..") == 0) {
            // if the curr name is ..

            if (curr->h_prev != NULL) {
                // back to the h_prev if h_prev is not null
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("cd: \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            Node find_node = lookInDir(curr->l_next, curr_token);
            
            if (find_node == NULL) {
                // if we didn't find the name
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("cd: \'%s\': No Such file or directory\n", path);
                return;
            } else {
                if (find_node->type == REGULAR_FILE) {
                        // if the find node is the file
                        // exit(not a dir) 
                        free (curr_token);
                        free (next_token);
                        free (path_string);
                        printf("cd: \'%s\': Not a directory\n", path);
                        return;
                } else {
                    // enter the find_node
                    curr = find_node;
                }
            }
        }
    }
    free (curr_token);
    free (next_token);
    free (path_string);
    // make the current directory the current node2
    fs->curr_dir = curr;
}

void FsLs(Fs fs, char *path) {
    // if the path is NULL
    if (path == NULL) {
        // make the curr the lower of the curr_dir
        Node curr = fs->curr_dir->l_next;
        while(curr != NULL) {
            // display the node on this level
            printf("%s\n", curr->name);
            curr = curr->next;
        }
        return;
    }

    Node curr = fs->curr_dir;
    char *path_string = strdup(path);
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }
        if (strcmp(curr_token, ".") == 0) {
            // if the curr name is "."
            // do nothing on the curr_dir
            curr = fs->curr_dir;
        } else if (strcmp(curr_token, "..") == 0) {
            // if the curr name is ".."
            if (curr->h_prev != NULL) {
                // if the h_prev is not null
                // back to h_prev
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("ls: \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            // if we didn't find the name
            Node find_node = lookInDir(curr->l_next, curr_token);
            if (find_node == NULL) {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("ls: \'%s\': No Such file or directory\n", path);
                return;
            } else {
                if (find_node->type == REGULAR_FILE) {
                        free (curr_token);
                        free (next_token);
                        free (path_string);
                        printf("ls: \'%s\': Not a directory\n", path);
                        return;
                } else {
                    // enter the find_node
                    curr = find_node;
                }
            }
        }
    }
    free (curr_token);
    free (next_token);
    free (path_string);
    // if the next name is null now
    // display the name under the curr_dir
    // that is the lower level of the curr node
    curr = curr->l_next;
    while(curr != NULL) {
        printf("%s\n", curr->name);
        curr = curr->next;
    }
}

void FsPwd(Fs fs) {
    print_current_dir(fs->curr_dir);
}

void FsTree(Fs fs, char *path) {
    if (path == NULL) {
        printf("/\n");
        tree(fs->root->l_next, 1);
        return;
    }
    Node curr = fs->curr_dir;
    char *path_string = strdup(path);
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }
        if (strcmp(curr_token, ".") == 0) {
            curr = fs->curr_dir;
        } else if (strcmp(curr_token, "..") == 0) {
            if (curr->h_prev != NULL) {
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("tree: \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            Node find_node = lookInDir(curr->l_next, curr_token);
            // if we didn't find the name
            if (find_node == NULL) {
                // the next name is null
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("tree: \'%s\': No Such file or directory\n", path);
                return;
            } else {
                if (find_node->type == REGULAR_FILE) {
                        free (curr_token);
                        free (next_token);
                        free (path_string);
                        printf("tree: \'%s\': Not a directory\n", path);
                        return;
                } else {
                    curr = find_node;
                }
            }
        }
    }
    free (curr_token);
    free (next_token);
    free (path_string);
    printf("%s\n", path);
    tree(curr->l_next, 1);
}

void FsPut(Fs fs, char *path, char *content) {
    // TODO
    Node curr = fs->curr_dir;
    char *path_string = strdup(path);
    char *curr_token = NULL;
    char *next_token = NULL;
    char *token = strtok(path_string, "/");
    if (token != NULL) {
        next_token = strdup(token);
    }
    while(next_token != NULL) {
        if(curr_token != NULL) {
            free(curr_token);
        }
        curr_token = next_token;
        token = strtok(NULL, "/");
        if (token != NULL) {
            next_token = strdup(token);
        } else {
            next_token = NULL;
        }

        // if the curr name is "."
        if (strcmp(curr_token, ".") == 0) {
            curr = fs->curr_dir;
        } else if (strcmp(curr_token, "..") == 0) {
            // if the curr name is ".."

            if (curr->h_prev != NULL) {
                // if the h_prev is not null
                // back to the upper dir
                curr = curr->h_prev;
            } else {
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("put: \'%s\': No Such file or directory\n", path);
                return;
            }
        } else {
            Node find_node = lookInDir(curr->l_next, curr_token);
            // didn't find the node
            if (find_node == NULL) {
                
                free (curr_token);
                free (next_token);
                free (path_string);
                printf("mkfile: cannot create directory \'%s\': No Such file or directory\n", path);
                return;
                
            } else {
                // find the node 
                if (find_node->type != REGULAR_FILE) {
                        if(next_token == NULL) {
                            free (curr_token);
                            free (next_token);
                            free (path_string);
                            printf("tree: \'%s\': Is a directory\n", path);
                            return;
                        } else {
                            curr = find_node;
                        }
                } else {
                    FILE *file = fopen(curr_token, "r+");
                    char *path_string = strdup(path);
                    char *curr_token = NULL;
                    char *next_token = NULL;
                    char *token = strtok(path_string, "/");
                    if (token != NULL) {
                        next_token = strdup(token);
                    }
                    while(next_token != NULL) {
                        if(curr_token != NULL) {
                            free(curr_token);
                        }
                        curr_token = next_token;
                        token = strtok(NULL, "");
                        if (token != NULL) {
                            next_token = strdup(token);
                        } else {
                            next_token = NULL;
                        }
                        int curr_tok;
                        curr_tok = atoi(curr_token);
		                putchar(curr_tok);
                    }
                    fclose(file);
                }    
            }
        }    

    }    
}

void FsCat(Fs fs, char *path) {
    // TODO
}

void FsDldir(Fs fs, char *path) {
    // TODO
}

void FsDl(Fs fs, bool recursive, char *path) {
    // TODO
}

void FsCp(Fs fs, bool recursive, char *src[], char *dest) {
    // TODO
}

void FsMv(Fs fs, char *src[], char *dest) {
    // TODO
}

//        helper functions         //

// create a new node
Node NewNode(char name[], FileType type) {
    Node node = malloc(sizeof(struct FsNode));
    node->name = strdup(name);
    node->h_prev = NULL;
    node->l_next = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->type = type;
    return node;
}

// free node
void NodeFree(Node node) {
    if (node != NULL) {
        NodeFree(node->next);
        NodeFree(node->l_next);
        free(node->name);
        free(node);
    }  
}

// look into the directory and check each node
Node lookInDir (Node node, char* name) {
    if (node == NULL) { 
        // not exist
        return NULL;
    } else if (strcmp(node->name, name) == 0) { 
        // find it
        return node;
    } else { 
        // check next
        return lookInDir(node->next, name);
    }
}

// initiate the node
Node create_here (Node node, Node prev, Node h_prev, char *name, FileType type) {
    if (node == NULL) {
        // create new node and initiate it
        Node new = NewNode(name, type);
        new->prev = prev;
        new->h_prev = h_prev;
        printf("created %s under %s ", name, new->h_prev->name);
        if (prev!= NULL) {            
            printf("after %s\n", new->prev->name);
        } else {
            printf("\n");
        }
        return new;
    } else if (strcmp(name, node->name) > 0) { 
        // order in canonical order
        // find until the next node satisfied the condition
        node->next = create_here (node->next, node, h_prev, name, type);
        return node;
    } else {
        // the node is not null and the name is in canonical order
        // create a new node and initiate it 
        Node new = NewNode(name, type);
        new->prev = prev;
        new->h_prev = h_prev;
        new->next = node;
        printf("created %s under %s ", name, new->h_prev->name);
        if (prev!= NULL) {            
            printf("after %s before %s\n", new->prev->name, new->next->name);
        } else {
            printf("before %s\n",new->next->name);
        }    
        return new;
    }
}

// prints the canonical path of the current working directory
void print_current_dir(Node curr) {
    char pwd[PATH_MAX];
    strcpy(pwd, "");
    char path[PATH_MAX + 1];
    while(curr != NULL) {
        // if the curr is not the root
        if(curr->h_prev != NULL) { 
            // attach the h_prev to the pwd
            strcpy(path, pwd);
            strcpy(pwd, "/");
            strcat(pwd, curr->name);
            strcat(pwd, path);
        }
        curr = curr->h_prev;
    }
    if (strcmp(pwd, "") == 0) {
        strcpy(pwd, "/");
    }
    printf("%s\n", pwd);
}

void tree(Node n, int level) {
    if (n == NULL) {
        return;
    }
    for (int i = 0; i < level; i++) {
        printf("    ");
    }
    printf("%s\n",n->name);
    tree(n->l_next, level + 1);
    tree(n->next, level);
    return;
}

