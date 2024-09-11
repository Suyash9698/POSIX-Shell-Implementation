#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/utsname.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <fcntl.h>
#include<csignal>
using namespace std;
string display="";
pid_t forePid=0;


void find(const string& input) {
   // std::string lowercaseInput = toLower(input); // Convert input to lowercase
    
    char* token;
    char* convert = strdup(input.c_str());
    token=strtok(convert," ");
    DIR* directory = opendir(".");
    if (directory == NULL) {
        cout << "Directory not found" << std::endl;
        return;
    }

    dirent* entries;
    entries = readdir(directory);
    string array[1000];
    int l = 0;

    while (entries != NULL) {
        array[l] = entries->d_name;
        l++;
        entries = readdir(directory);
    }

    closedir(directory);
    std::string res[1000];
    int k = 0;
    for(int i=0;i<l;i++){
        if(array[i].size()<input.size()){
            continue;
        }
        string sub=array[i].substr(0,input.size());
        if(strcmp(sub.c_str(),input.c_str())==0){
            res[k]=array[i];
            k++;
        }
    }
    if(k==0){
        cout<<"No Matching Found"<<endl;
        return;
    }
    if (k == 1) {
        cout << res[0] << endl;
        return;
    } else {
        for (int i = 0; i < k; i++) {
            cout << res[i] << endl;
        }
    }
}
//this is for handling the ctrl+z, ctrl+d, ctrl+c  
void handler(int signal){
     //for ctrl+z
     //cout<<"hggg"<<endl;
    if(signal==SIGTSTP){
        if(forePid!=0){
            kill(-forePid,SIGTSTP);
          //  forePid=0;
           
        }
    }
    //for ctrl+c
    else if(signal==SIGINT){
        if(forePid!=0){
           kill(-forePid,SIGINT);
           int status;
           pid_t pid;
           pid=waitpid(pid,&status,0);
           if(pid!=-1){
            if(WIFSIGNALED(status)||WIFEXITED(status)){
                //resetting the status of forePid=0
                forePid=0;
            }
           }
           
        }

    }
    return;
   
}
void ioRedirection(string& input) {
    char* token;
    char* convert = strdup(input.c_str());
    // determinining if the input includes redirection operators
    bool hasInputRedirection = (input.find("<") != string::npos);
    bool hasOutputRedirection = (input.find(">>") != string::npos) || (input.find(">") != string::npos);
    //fd1 is for input file
    int fd1 = -1;
    //fd2 is for output file
    int fd2 = -1;
    token = strtok(convert, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                // opening the input file
                //in read only mode
                fd1 = open(token, O_RDONLY);
                if (fd1 == -1) {
                    cout<<"error: Unable to open input file"<<endl;
                    exit(0);
                }
            }
        } 
        else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                fd2 = open(token, O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (fd2 == -1) {
                    cout<<"error: Unable to open output file"<<endl;
                    exit(0);
                }
            }
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                fd2 = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd2 == -1) {
                    cout<<"error: Unable to open output file"<<endl;
                    exit(0);
                }
            }
        } else {
            token = strtok(NULL, " ");
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // inside the child process
        if (hasInputRedirection) {
            if (fd1 != -1) {
                //here dup2 used for redirecting the fd1 to input
                dup2(fd1, 0); 
                close(fd1);  
            }
        }
        if (hasOutputRedirection) {
            if (fd2 != -1) {
                //here dup2 used for redirecting the fd2 to output
                dup2(fd2, 1); 
                close(fd2);  
            }
        }
        system(input.c_str());
        _exit(EXIT_SUCCESS);
    } else if (pid > 0) {
        // inside the parent process
        wait(NULL);
        if (fd1 != -1) {
            close(fd1);  
        }
        if (fd2 != -1) {
            close(fd2); 
        }
    }
}



//this code is for linux/windows users not for mac
void processGetInfo(const string& pid) {
    // First, we will find the process status using /proc + pid + /status
    
    cout << pid << endl;
    string path = "/proc/" + pid + "/status";
  //  string path1 = path + "/status";
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        //cout << "Error: unable to open the path of status of process" << endl;
        cerr << "Error opening the path of the status of the process: " << strerror(errno) << endl;
        
        return;
    }
    // Now reading and storing process
    char buffer[8192];
    ssize_t bytesRead;
    while (true) {
        bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            break;
        }
        string val(buffer, bytesRead);
        if (val.find("State:") != string::npos) {
            // Finding the newline after state
            size_t line = val.find('\n', val.find("State:"));
            if (line != string::npos) {
                size_t temp = val.find("State:");
                size_t len = line - temp - 7;
                string ans = val.substr(temp + 7, len + 1);
                cout << ans << endl;
                break;
            }
        }
    }
    close(fd);
}

void searchingInDirectory(string& input) {
    char* token;
    char* convert;
    convert=strdup(input.c_str());
    token=strtok(convert," ");
    token=strtok(NULL," ");
    char dirpath[2000];
    if (getcwd(dirpath, sizeof(dirpath)) == NULL) {
        std::cerr << "Error getting current working directory" << std::endl;
        return;
    }

    DIR* direc = opendir(dirpath);
    if (direc == NULL) {
        std::cerr << "Invalid directory" << std::endl;
        return;
    }
    dirent* dirEntries;
    dirEntries = readdir(direc);
    //if(dirEntries->d_name==name)
    
    while(dirEntries != NULL) {
        if (strcmp(dirEntries->d_name, token) == 0){
            cout << "True" << endl;
            closedir(direc);
            return;
        }
       // cout<<dirEntries->d_name<<endl;
        dirEntries = readdir(direc);
    }

    cout << "False" << endl;
    closedir(direc);
}
void handleBackground(string& input){
    // for this you need to open a new process
    pid_t pid = fork();
    if (pid == -1) {
        cout<<"error: unable to create a new process"<<endl;
    } 
    else if (pid == 0) {
        // To start the child process as a new process
        int val = system(input.c_str());
        _exit(val); 
    } 
    else if(pid > 0){
        cout <<pid<<endl;
    }
}
void handleForeground(string& input){
    pid_t pid;
    pid=fork();
    if(pid==0){
        int val=system(input.c_str());
        _exit(val);
    }
    else if(pid>0){
        forePid=pid;
        int x;
        waitpid(pid,&x,0);
        forePid=0;
    }
}
void helpLs(string& dirpath,bool onlyls,bool hidden,bool longFormat){
    DIR* directory=opendir(dirpath.c_str());
    if(directory==NULL){
        cout<<"Invalid directory"<<endl;
        return;
    }
    //to store the files and subdirectory
    // we have struct dirent
    dirent* dirEntries;
    dirEntries=readdir(directory);
    //only for ls command
    if(onlyls==true){
     while(dirEntries!=NULL){
        if(dirEntries->d_name[0]=='.'){
            dirEntries=readdir(directory);
        }
        else{
            cout<<dirEntries->d_name<<"         ";
            dirEntries=readdir(directory);
        }
     }
    }
    //this is for ls -la or ls -al
    else if(hidden==true && longFormat==true){
        while(dirEntries!=NULL){
        string filePath=dirpath+"/"+dirEntries->d_name;
        struct stat dirInfo;
        stat(filePath.c_str(),&dirInfo);
        string whole="";
        mode_t permissions=dirInfo.st_mode & 0777;
        whole+=(dirInfo.st_mode & (S_IFDIR == 0)) ? "-":"d";
        //for users
        whole+=permissions & 0400?"r":"-";
        whole+=permissions & 0200?"w":"-";
        whole+=permissions & 0100?"x":"-";
        //for groups
        whole+=permissions & 0040?"r":"-";
        whole+=permissions & 0020?"w":"-";
        whole+=permissions & 0010?"x":"-";
        //for others
        whole+=permissions & 0004?"r":"-";
        whole+=permissions & 0002?"w":"-";
        whole+=permissions & 0001?"x":"-";
        time_t modificationTime = dirInfo.st_mtime;

        // Convert the modification time to a struct tm
        struct tm* timeInfo = localtime(&modificationTime);
        const char* months[] = {
                   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        // Extract components from timeInfo
        const char* month = months[timeInfo->tm_mon];
        int day = timeInfo->tm_mday;
        int hour = timeInfo->tm_hour;
        int minute = timeInfo->tm_min;
        struct passwd* host;
        host= getpwuid(dirInfo.st_uid);
        struct group* gname;
        gname=getgrgid(dirInfo.st_gid);
       cout<<whole<<" "<<dirInfo.st_nlink<<" "<<host->pw_name<<" "<<gname->gr_name<<" "<<dirInfo.st_size<<" "<<month<<" "<<day<<" "<<
                    hour<<":"<<minute<<" "<<dirEntries->d_name<<endl;
       dirEntries=readdir(directory);
      }
    }
    //this is for ls -a
    else if(hidden==true){
        while(dirEntries!=NULL){
            cout<<dirEntries->d_name<<"             ";
            dirEntries=readdir(directory);
        }
    }
    //for ls-l
    else if(longFormat==true){
      while(dirEntries!=NULL){
        if(dirEntries->d_name[0]=='.'){
            dirEntries=readdir(directory);
        }
        else{
        string filePath=dirpath+"/"+dirEntries->d_name;
        struct stat dirInfo;
        stat(filePath.c_str(),&dirInfo);
        string whole="";
        mode_t permissions=dirInfo.st_mode & 0777;
        whole+=(dirInfo.st_mode & (S_IFDIR == 0)) ? "-":"d";
        //for users
        whole+=permissions & 0400?"r":"-";
        whole+=permissions & 0200?"w":"-";
        whole+=permissions & 0100?"x":"-";
        //for groups
        whole+=permissions & 0040?"r":"-";
        whole+=permissions & 0020?"w":"-";
        whole+=permissions & 0010?"x":"-";
        //for others
        whole+=permissions & 0004?"r":"-";
        whole+=permissions & 0002?"w":"-";
        whole+=permissions & 0001?"x":"-";
        time_t modificationTime = dirInfo.st_mtime;

        // Convert the modification time to a struct tm
        struct tm* timeInfo = localtime(&modificationTime);
        const char* months[] = {
                   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        // Extract components from timeInfo
        const char* month = months[timeInfo->tm_mon];
        int day = timeInfo->tm_mday;
        int hour = timeInfo->tm_hour;
        int minute = timeInfo->tm_min;
        struct passwd* host;
        host= getpwuid(dirInfo.st_uid);
        struct group* gname;
        gname=getgrgid(dirInfo.st_gid);
       cout<<whole<<" "<<dirInfo.st_nlink<<" "<<host->pw_name<<" "<<gname->gr_name<<" "<<dirInfo.st_size<<" "<<month<<" "<<day<<" "<<
                    hour<<":"<<minute<<" "<<dirEntries->d_name<<endl;
       dirEntries=readdir(directory);
        }
      }
    }
}
void handlingLs(string& input){
     char* token;
     char* convert;
     convert=strdup(input.c_str());
     token=strtok(convert," ");
     token=strtok(NULL," ");
     //default directory
     string directoryPath=".";
     bool longFormat=false;
     bool hidden=false;
     bool onlyls=false;
     //onlyls -> hidden -> longFormat
    //things to do
            if(token==NULL){
                helpLs(directoryPath,true,false,false);
            }
            else if(strcmp(token,"..")==0){
                directoryPath="..";
                helpLs(directoryPath,true,false,false);
            }
            else if(strcmp(token,".")==0){
                directoryPath=".";
                helpLs(directoryPath,true,false,false);
            }
            else if(strcmp(token,"~")==0){
                directoryPath=getenv("HOME");
                helpLs(directoryPath,true,false,false);
            }
            else if(strcmp(token,"-a")==0){
                token=strtok(NULL," ");
                //for ls -a -l
                if(token!=NULL && strcmp(token,"-l")==0){
                     token=strtok(NULL," ");
                     if(token!=NULL){
                        directoryPath=token;
                     }
                    helpLs(directoryPath,false,true,true);
                }
                else{
                   helpLs(directoryPath,false,true,false);
                }
            }
            else if(strcmp(token,"-l")==0){
                token=strtok(NULL," ");
                //for ls -a -l
                if(token!=NULL && strcmp(token,"-a")==0){
                     token=strtok(NULL," ");
                     if(token!=NULL){
                        directoryPath=token;
                     }
                    helpLs(directoryPath,false,true,true);
                }
                else{
                    helpLs(directoryPath,false,false,true);
                }
            }
            else if((strcmp(token,"-al")==0)||(strcmp(token,"-la")==0)){
                 token=strtok(NULL," ");
                if(token!=NULL){
                    directoryPath=token;
                }
                helpLs(directoryPath,false,true,true);
            }
            //means file/directory name is there
            else if(token!=NULL){
                directoryPath=token;
                helpLs(directoryPath,true,false,false);
            }
     cout<<endl;
}
void handlingWithCd(string& input){
    char* token;
    char* convert;
    token=strtok(&input[0]," ");
    token=strtok(NULL," ");
    if(token==NULL || strcmp(token,".")==0){
        return;
    }
    if (strcmp(token,"-")==0) {
        //cout<<"okk";
        const char* path = getenv("OLDPWD");
        if (path == NULL) {
            cout << "Not found old pwd" << endl;
            return;
        }


        int val = chdir(path);
        if (val == -1) {
            cerr << "error: Unable to change directory to the previous" <<endl;
        }
       
    }
    else if(strcmp(token,"~")==0){
        string path=getenv("HOME");
        int val=chdir(path.c_str());
        if(val==-1){
          cout<<"error: in changing directory b"<<endl;
        }
    }
    else{
        char cwd[2000];
        if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        perror("getcwd");
        return;
        }
        
        if (chdir(token) == -1) {
        perror("chdir");
        return;
        }
     
    }
     char cwd2[2000];
     getcwd(cwd2,sizeof(cwd2));
    
     string curr=cwd2;
    string homeDir=getenv("HOME");
    //checking the prefix of cwd and home
    bool flag=true;
    for(size_t i=0;i<homeDir.length();i++){
        if(homeDir[i]!=curr[i]){
            flag=false;
            break;
        }
    }
    if(flag==false){
        cout<<"error ho gaya jii"<<endl;
        exit(1);
    }
    
    char hostname[200];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        cerr << "Unable to Retrieve system name" <<endl;
        exit(1);
    }
    
    string pwd="~"+curr.substr(homeDir.length());
    display=getlogin()+string("@")+hostname+":"+pwd+">";
}
string DisplayWithoutCd(){
        char cwd[8192];
        struct utsname uts;
        string homeDir=getenv("HOME");
        //want to get the cwd also
        getcwd(cwd,sizeof(cwd));
        string curr=cwd;
        //checking the prefix of cwd and home
        bool flag=true;
        for(size_t i=0;i<homeDir.length();i++){
            if(homeDir[i]!=cwd[i]){
                flag=false;
                break;
            }
        }
        if(flag==false){
            cerr<<"error"<<endl;
            exit(1);
        }
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) != 0) {
           cerr << "Unable to Retrieve system name" <<endl;
           exit(1);
        }
        string pwd="~"+curr.substr(homeDir.length());
        display=getlogin()+string("@")+hostname+":"+pwd+">";
        return display;
}
int main(void){
    char buffer[200];
    bool first=true;
    signal(SIGTSTP,handler);
    signal(SIGINT,handler);
    
    while(true){
        if(first==true){
            display=DisplayWithoutCd();
        }
        cout<<display;
        //now user will give the inputs whatsoever
        string input;
        getline(cin,input);
        string here="";
        bool tab=false;
        for(char ch:input){
            if(ch=='\t'){
                find(here);
                tab=true;
                break;
            }
            else{
                here+=ch;
            }
        }
        if(tab==true){
            continue;
        }
         if (cin.eof()) {
            // Start a new shell using the 'system' function
            cout<<endl;
            exit(0);
        }
        if(input=="exit"){
            break;
        } 
        //ioRedirection(input);  
        //break;  
        //cd case
        char* token;
        char* convert;
        convert=strdup(input.c_str());
        token=strtok(convert," ");
        bool hasInputRedirection = (input.find("<") != string::npos);
        bool hasOutputRedirection = (input.find(">>") != string::npos) || (input.find(">") != string::npos);
        if(hasInputRedirection==true || hasOutputRedirection==true){
            ioRedirection(input);
            continue;
        }
        if(strcmp(token,"cd")==0){
            handlingWithCd(input);
            first=false;
        }
        //background processes
        char* token2;
        token2=strtok(convert,"&");
        if(token2!=NULL && token2 != convert){
            
            handleBackground(input);
        }

        //present working directory
        else if(strcmp(token,"pwd")==0){
           char buffer[1000];
           getcwd(buffer,sizeof(buffer));
           cout<<buffer<<endl;
        }
        //echo implement
        else if(strcmp(token,"echo")==0){
            while(token!=NULL){
                token=strtok(NULL," ");   
                if(token!=NULL){
                    cout<<token<<" ";
                }
            }
            cout<<endl;
        }
       else if(strcmp(token,"ls")==0){
            handlingLs(input);
        }
        else if(strcmp(token,"search")==0){
            searchingInDirectory(input);
        }
        else if(strcmp(token,"pinfo")==0){
            pid_t pid;
            pid = getpid();
            processGetInfo(to_string(pid));
        }
        else{
            handleForeground(input);
        }
    }
}