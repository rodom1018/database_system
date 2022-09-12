#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <iostream>

//functions
void print_main_menu();
std::string insert_metadata();
std::string make_one_block(std::string sample);
std::string make_three_letter(std::string sample);
bool avail_free_space(std::string info , int len);

//common variables
int ONE_LINE = 300;
int PAGE_SIZE = ONE_LINE + 2;

//구현해야할 내용 =  테이블 생성, 레코드 삽입, 레코드 검색, 칼럼 검색.

//map (table name -> directory)
std::map<std::string, std::string> table_info;
std::string prev_record = "";

int main(){
    char buffer[PAGE_SIZE]; // assume limited buffer size.
    print_main_menu();

    int now_menu=1;
    scanf("%d", &now_menu);

    FILE * fp;
    if(now_menu == 1) {
        //create table
        printf("type table name!\n");
        std::string now_finding;
        std::cin >> now_finding;
        std::string temp_directory = ".\\tables\\" + now_finding + ".txt";

        if (fp = fopen(temp_directory.c_str(), "r")) {
            //이미 테이블 이름이 있음 ( 못만든다)
            printf("table name already exists.... try something else!\n");
        } else {
            //새로운 테이블 이름
            fp = fopen(temp_directory.c_str(), "w");
            std::string now_meta = insert_metadata();
            fwrite(now_meta.c_str(), now_meta.length(), 1, fp);

            std::string blank = "000299";
            blank = make_one_block(blank);
            for(int i=0; i<100; i++) {
                fwrite(blank.c_str(), blank.length(), 1, fp);
            }
        }
    }else if(now_menu == 2){
        //레코드 삽입
        printf("type table name!\n");
        std::string now_finding;
        std::cin >> now_finding;
        std::string temp_directory = ".\\tables\\" + now_finding +".txt";
        fp = fopen(temp_directory.c_str(), "r+");

        //see metadata, and look how many columns in the table.
        fread(buffer, PAGE_SIZE, 1, fp);
        int num_col = buffer[0] - '0';

        std::string store[num_col];

        for(int i=1 ; i<=num_col ; i++){
            printf("%d (st/nd/rd/th) column value ? :\n", i);
            std::cin >> store[i-1];
        }

        //find the first
        //fread

        std::string left =""; // 6byte
        std::string right = ""; // ?

        //jump = sees metadata and check the col is varchar ? or not ?
        int jump = num_col +1;
        for(int i=1 ; i<=num_col ; i++){
            jump +=(buffer[i]-'0');
        }

        //가변길이 레코드 포맷 varchar start point.,
        int start_varchar = 1;
        for(int i=1 ; i<=num_col; i++){
            if(buffer[jump+i] == '0'){
                //char
                start_varchar += (buffer[jump+i+num_col] - '0');
            }else{
                //varchar
                start_varchar += 3;
            }
        }

        for(int i=1; i<=num_col ;i++){
            if(buffer[jump+i] == '0'){
                right += store[i-1];
            }else{
                right += std::to_string(start_varchar);
                int length = store[i-1].length();
                right += std::to_string(length);
                start_varchar += length;
            }
        }

        right +=  "0"; // null bitmap

        for(int i=1 ; i<=num_col; i++){
            if(buffer[jump+i] =='1' ){
                right += store[i-1];
            }
        }
        //std::cout << right << std::endl; //00001M14241630cskim

        int p = 1;
        while(true) {
            rewind(fp);
            fseek(fp, PAGE_SIZE * p, SEEK_SET);
            fread(buffer, ONE_LINE, 1, fp);

            //std::cout << buffer <<std::endl;

            std::string now_buffer(buffer);
            if(avail_free_space(now_buffer, right.length())){
                std::string sub_str = now_buffer.substr(3,3);
                int end_free_space = std::stoi(sub_str);

                std::string sub_str2 = now_buffer.substr(0,3);
                int start_free_space = (std::stoi(sub_str2)+1) * 6;

                left += make_three_letter(std::to_string(right.length()));
                left += make_three_letter(std::to_string(end_free_space - right.length()));

                now_buffer.replace(start_free_space, 6, left);
                now_buffer.replace(end_free_space - right.length() + 1, right.length(), right);
                now_buffer.replace(3,3,std::to_string(end_free_space - right.length()));
                now_buffer.replace(0,3 ,make_three_letter(std::to_string(std::stoi(sub_str2)+1)));

                rewind(fp);
                fseek(fp, PAGE_SIZE * p, SEEK_SET);
                fwrite(now_buffer.c_str(), now_buffer.length(), 1, fp);
                break;
            }

            p = p+1;
        }


    }else if(now_menu == 3){
        //레코드 검색
        printf("type table name!\n");
        std::string now_finding;
        std::cin >> now_finding;
        std::string temp_directory = ".\\tables\\" + now_finding +".txt";
        fp = fopen(temp_directory.c_str(), "r+");

        printf("type id that you want!\n");
        int find_id;
        scanf("%d", &find_id);

        int p = 1;
        int find_flag = 0;
        while(true){
            rewind(fp);
            fseek(fp, PAGE_SIZE * p, SEEK_SET);
            fread(buffer, ONE_LINE, 1, fp);
            std::string now_buffer(buffer);

            int n = std::stoi(now_buffer.substr(0,3)); // how many records in that block?
            if(n==0) break;
            for(int i=1; i<=n ; i++) {
                int now_size = std::stoi(now_buffer.substr(6*i, 3));
                int now_start = std::stoi(now_buffer.substr(6*i+3,3));
                //std::cout << now_buffer.substr(now_start+1, 5) << std::endl;
                int now_id = std::stoi(now_buffer.substr(now_start+1, 5));

                if(now_id == find_id){
                    prev_record = now_buffer.substr(now_start+1, now_size);
                    find_flag = 1;
                    break;
                }
            }

            if(find_flag == 1) break;
            p = p+1;
        }

        int flag_column_search = 0;
        if(find_flag == 1) {
            std::cout << "Found Record! Record is  :: " << std::endl;
            std::cout << prev_record << "  ." << std::endl;
            std::cout << "Continue with searching column ? (yes = 1 , no = 0)" << std::endl;
            std::cin >> flag_column_search;
        }else{
            std::cout << "Cannot Find record. " <<std::endl;
        }

        if(flag_column_search == 1){
            std::string search_col ;
            std::cout << "Input the column name that you want." << std::endl;
            std::cin >> search_col;

            rewind(fp);
            fread(buffer, ONE_LINE, 1, fp);
            std::string now_buffer(buffer);

            int n = std::stoi(now_buffer.substr(0,1));

            int place = -1; // 몇번째  col 하고 일치 ?
            int p = n+1;
            for(int i=1 ; i<=n ; i++){
                int now_len = std::stoi(now_buffer.substr(i,1));
                if(now_len == search_col.length()){
                    if(now_buffer.substr(p, now_len).compare(search_col) == 0){
                        place = i;
                        break;
                    }
                }
                p+=now_len;
            }

            if(place == -1){
                std::cout << "you typed incorrect col name . " << std::endl;
            }else {

                //jump = sees metadata and check the col is varchar ? or not ?
                int jump = n+1;
                for(int i=1 ; i<=n; i++){
                    jump +=(buffer[i]-'0');
                }

                int col_p = 0;
                for (int i = 1; i < place; i++) {
                    if (buffer[jump + i] == '0') {
                        //char
                        col_p += (buffer[jump+n+i] - '0');
                    } else {
                        //varchar
                        col_p += 3;
                    }
                }
                //std::cout << prev_record << std::endl;
                std::string col_result ;
                int null_bit = buffer[jump];
                //printf("%d", null_bit);
                //printf("%d!!\n",null_bit >>place);
                if((null_bit >>place & 1) == 1){
                    //null bitmap
                    col_result = "null";
                }else{
                    if(buffer[jump+place] == '0'){
                        //char
                        int temp = buffer[jump+n+place]-'0';
                        col_result = prev_record.substr(col_p, temp);
                    }else{
                        //varchar
                        int start =std::stoi(prev_record.substr(col_p,2));
                        int end =  std::stoi(prev_record.substr(col_p+2,1));
                        col_result = prev_record.substr(start , end);
                    }
                }

                std::cout <<"column value is :: " <<col_result << std::endl;
            }

        }

    }

    fclose(fp);
    return 0;
}

void print_main_menu(){
    printf("===================================\n");
    printf("Insert Number!\n");
    printf("1. Create Table\n");
    printf("2. Insert record\n");
    printf("3. Search record & Search Column\n");
    printf("===================================\n");
}

bool avail_free_space(std::string info , int len){
    std::string sub_str = info.substr(3,3);
    int end_free_space = std::stoi(sub_str);

    std::string sub_str2 = info.substr(0,3);
    int start_free_space = (std::stoi(sub_str2)+1) * 6;
    //printf("%d %d\n", end_free_space, start_free_space);

    int remain = end_free_space - start_free_space;

    if(remain > len + 6) return true;
    else return false;
}

std::string make_one_block(std::string sample){
    int remain = ONE_LINE - sample.length(); //
    sample.append(remain, ' ');
    sample += "\n";

    return sample;
}

std::string make_three_letter(std::string sample){
    int len = sample.length();

    if(len == 1){
        sample = "00" + sample ; // 공백 2 + 본론
    }else if(len ==2 ){
        sample = "0" + sample; // 공백 1 + 본론
    }

    return sample;
}

std::string insert_metadata(){
    //write metadata
    std::string metadata = "";

    int num_data;
    printf("How many columns ? \n");
    scanf("%d", &num_data);
    metadata += std::to_string(num_data);

    for(int i=1; i<=num_data ;i++){
        int temp;
        printf("%d (st/nd/rd/th) column's length :\n", i);
        scanf("%d", &temp);

        metadata += std::to_string(temp);
    }

    for(int i=1; i<=num_data ;i++){
        std::string temp;
        printf("%d (st/nd/rd/th) column's name:\n",i);
        std::cin >> temp;
        metadata += temp;
    }

    metadata += "0"; // null bitmap

    for(int i=1; i<=num_data ; i++){
        std::string temp;
        printf("%d (st/nd/rd/th) column is varchar ? (yes = 1 no = 0) :\n", i );
        std::cin >> temp;
        metadata +=temp;
    }

    for(int i=1; i<=num_data ; i++){
        std::string temp;
        printf("%d (st/nd/rd/th) column length?(data value length) (varchar = 0) : \n", i);
        std::cin >> temp;
        metadata += temp;
    }

    metadata = make_one_block(metadata);

    return metadata;
}