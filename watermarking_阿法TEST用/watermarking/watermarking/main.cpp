#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <algorithm>
#include<math.h>
#include<vector>
#include <map>
#include<stack>
#include<time.h>
using namespace std;


string line;
int input_bit = 0;
int output_bit = 0;
int transition_num = 0;
int state_num = 0;//state 數量
int start_state = 0;
//int input_num = 0;//2^(input_bit)
int watermark_num;//watermark組數
struct Watermark{
	vector<string> watermark_input;//watermark input部分
	vector<string> watermark_output;//watermark output部分
};
Watermark wm[3];
int ourStart_state = -1;
int env = 0;
vector<string> ini;
struct NextNode {
	int next_state;
	string output;
};
vector<map<string, NextNode> > vecState;//以STATE_NAME作為INDEX存放mapNextState
vector<map<string, NextNode> > origin_vecState;//存放最原始的FSM
const string md5_file[3] = { "md5_1","md5_2","md5_3" };


//<function header>
void connect(int);
bool isINCLUDE(string, string);
string counter_add(string);
void read_watermark(int);
void output_data(int);
string isComplete(int);
int find_ourStart_state(int);
//</function header>

bool isCSFSM() {
	for (int i = 0; i < origin_vecState.size(); i++) {
		if (isComplete(i) != "Complete") {
			return false;
		}
	}
	return true;//complete
}

string isComplete(int currentState) {
	string counter = "";
	counter.resize(input_bit);//設置courter大小 = .i
	counter.assign(input_bit, '0');
	string counter_end = "";
	counter_end.resize(input_bit);//設置courter大小 = .i
	counter_end.assign(input_bit, '0'); //111 + 1 ->000 
	bool start_loop = true;
	map<string, NextNode>::iterator it = origin_vecState[currentState].begin();
	while (counter != counter_end || start_loop) {//if (counter == 1111111) --> 比下一組 //start_loop 讓000=000可以進來
		if (isINCLUDE(counter, it->first)) {//相等
			counter = counter_add(counter);	//用下一組的counter去比較(counter+1)
			start_loop = false; // 鎖住
			it = origin_vecState[currentState].begin();//回到開始點
		}
		else {
			it++;//比較下一個input
			if (it == origin_vecState[currentState].end()) {
				return counter;//有dont care 所以不是complete
			}
		}
	}
	return "Complete";
}

string counter_add(string counter) {//棒棒的
	if (counter[counter.size() - 1] == '0') {
		counter[counter.size() - 1] = '1';
		return counter;
	}
	else {//最後一個是1
		if (counter.size() > 1) {
			return counter_add(counter.substr(0, counter.size() - 1)) + "0";
		}
		else {//111-->000
			return "0";
		}
	}
}

bool isINCLUDE(string watermark, string FSM_input) {//DON'T CARE包含這組INPUT
	for (int i = 0; i < input_bit; i++) {
		if (FSM_input[i] != '-') {
			if (FSM_input[i] != watermark[i])
				return false;
		}
	}
	return true;
}

void make_table() {
	//input_num = pow(2, input_bit);
	for (int i = 0; i < state_num; i++) {//建立大小為s的vector
		map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
		vecState.push_back(mapNextState);
	}
}

void make_origin_table() {
	//input_num = pow(2, input_bit);
	for (int i = 0; i < state_num; i++) {//建立大小為s的vector
		map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
		origin_vecState.push_back(mapNextState);
	}
}


void save_origin_table(string input_line, string state_from_line, string next_state_line, string output_line) {
	int state_index = stoi(state_from_line.substr(1, state_from_line.size() - 1));//抓state_from的string改成int
	int next_state_int = stoi(next_state_line.substr(1, next_state_line.size() - 1));//抓next_state的string改成int
	NextNode next_node;
	next_node.next_state = next_state_int;
	next_node.output = output_line;

	//將key "input"與value "next_node" 放入map
	origin_vecState[state_index].insert(pair<string, NextNode>(input_line, next_node));//也可以寫成origin_vecState[state_index][input_line]=next_node;

}

void save_table(string input_line, string state_from_line, string next_state_line, string output_line) {
	int state_index = stoi(state_from_line.substr(1, state_from_line.size() - 1));//抓state_from的string改成int
	int next_state_int = stoi(next_state_line.substr(1, next_state_line.size() - 1));//抓next_state的string改成int
	NextNode next_node;
	next_node.next_state = next_state_int;
	next_node.output = output_line;

	//將key "input"與value "next_node" 放入map
	vecState[state_index].insert(pair<string, NextNode>(input_line, next_node));//也可以寫成vecState[state_index][input_line]=next_node;
}

void connect(int a) {//暫時
	//找起始點
	ourStart_state = find_ourStart_state(start_state);//找到一個有dont care的node
	string newRoad = isComplete(ourStart_state);

	//從我可走的點出去 = ourStart_state
	string  ourStart = "S" + to_string(ourStart_state);
	string our_nextState = "S" + to_string(origin_vecState.size());
	map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
	origin_vecState.push_back(mapNextState);
	save_origin_table(newRoad, ourStart, our_nextState, wm[a].watermark_output[0]);
	ourStart = our_nextState;
	transition_num++;
	state_num++;
	//按watermarking直接生出所有tr和state ,放在origin_vecState中
	for (int i = 0; i < wm[a].watermark_input.size(); i++) {
		our_nextState = "S" + to_string(origin_vecState.size());
		map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
		origin_vecState.push_back(mapNextState);
		save_origin_table(wm[a].watermark_input[i], ourStart, our_nextState, wm[a].watermark_output[i]);
		ourStart = our_nextState;

		transition_num++;
		state_num++;
	}
	//再把origin_vecState 印出//印出md5_env
	output_data(a);
}

int find_ourStart_state(int currentState) {
	stack<int> sk;
	int *it_counter=new int [origin_vecState.size()];//計算用過的路的數量
	for (int i = 0; i < origin_vecState.size(); i++) {
		it_counter[i] = 0;
	}
	map<string, NextNode>::iterator it = origin_vecState[currentState].begin();
	sk.push(currentState);
	
	while (1) {//map
		if (it_counter[currentState] != 0 || it == origin_vecState[currentState].end()) {//如果current state有走過，就要pop回去
			//pop
			if(!sk.empty())
				sk.pop();
			currentState = sk.top();
			ini.pop_back();
			
			//it指向上一個STATE的下一條路
			it = origin_vecState[currentState].begin();
			for (int i = 0; i < it_counter[currentState];i++)
				it++;
		}
		if (it != origin_vecState[currentState].end()) {
			if (isComplete(currentState) != "Complete") {//有dont care
				return currentState;//state
			}
			else {//complete
				it_counter[currentState]++;//這個STATE的it counter++
				currentState = it->second.next_state;
				sk.push(currentState);
				ini.push_back(it->first);
				it = origin_vecState[currentState].begin();
			}
		}
	}
}

void show_table() {
	map<string, NextNode>::iterator iter;
	map<string, NextNode>::reverse_iterator iter_r;
	for (int i = 0; i < state_num; i++) {
		cout << "S" << i << ": ";
		for (iter = origin_vecState[i].begin(); iter != origin_vecState[i].end(); iter++) {
			cout << iter->first << " ";
		}
		cout << endl;
	}
	cout << endl << endl;
}

void read_kiss(string file_name) {
	fstream fin;
	fin.open(file_name, ios::in);
	if (!fin) {
		throw "read kiss fail!";
	}
	else {
		while (getline(fin, line)) {
			stringstream ss(line);
			if (line[0] == '0' || line[0] == '1' || line[0] == '-') {//抓到transition
				string input, state_from, state_to, output;
				ss >> input >> state_from >> state_to >> output;

				save_origin_table(input, state_from, state_to, output);//存到MAP裡面


				int count_include = 0;
				int a = -1;
				a = input.find('-');
				if (a != -1) {//有don't care
					for (int i = 0; i < watermark_num; i++) {
						for (int file_a = 0; file_a < 3; file_a++) {
							if (isINCLUDE(wm[file_a].watermark_input[i], input)) {
								count_include++;
								save_table(wm[file_a].watermark_input[i], state_from, state_to, output);//存到MAP裡面
								//cout << "watermark_input[" << i << "]: " << wm[file_a].watermark_input[i] << endl;
								//cout << " " << input << " " << state_from << " " << state_to << " " << output << endl;
							}
							if (i == watermark_num - 1 && count_include == 0) {//如果沒有watermark符合這個input,我們就自己隨便選一條路加入MAP
								for (int current_bit = 0; current_bit < input.size(); current_bit++) {
									if (input[current_bit] == '-')
										input[current_bit] = '0';
								}
								//cout<< "no include: "<< input << " " << state_from << " " << state_to << " " << output << endl;
							}
						}
					}
				}
				else {//沒有don't care
					save_table(input, state_from, state_to, output);//存到MAP裡面
				}

				int count_dontcare = 0;
				for (int i = 0; i < input.size(); i++) {
					if (input[i] == '-')
						count_dontcare++;
				}
			}
			else if (line[0] == '.') {
				string s;
				switch (line[1]) {
				case 'e':
					break;
				case 'i':
					ss >> s >> input_bit;
					break;
				case 'o':
					ss >> s >> output_bit;
					break;
				case 'p':
					ss >> s >> transition_num;
					break;
				case 's':
					ss >> s >> state_num;
					break;
				case 'r':
					ss >> s >> start_state;
					make_table();
					make_origin_table();
					for (int a = 0; a < 3; a++)
						read_watermark(a);
					break;
				}
			}
			ss.clear();
		}
	}
}

string changeHEXtoBIN(string HEX_watermark) {
	string temp_BIN_watermark = "";
	transform(HEX_watermark.begin(), HEX_watermark.end(), HEX_watermark.begin(), tolower);//大換小英文字
	for (int i = 0; i < HEX_watermark.size(); i++)
	{
		int num;
		if (HEX_watermark[i] >= 'a' && HEX_watermark[i] <= 'f')
			num = HEX_watermark[i] - 'a' + 10;//16->10進位
		else
			num = HEX_watermark[i] - '0';

		string bin;
		while (num > 0) {//2進
			bin = to_string(num % 2) + bin;//把最後的存到前面去
			num /= 2;
		}

		while (bin.size() < 4) { //補0 bit
			bin = "0" + bin;
		}
		temp_BIN_watermark += bin;
	}

	cout << temp_BIN_watermark << endl;

	//若128除以(input_bit + output_bit)有餘數,補0
	int remainder = 128 % (input_bit + output_bit);
	if (remainder != 0) {
		int lack = input_bit + output_bit - remainder;//缺多少bit
		for (int i = 0; i < (input_bit + output_bit); i++) {
			temp_BIN_watermark += "0";
		}
	}

	return temp_BIN_watermark;
}
string changeIntToHEX(int dec) {
	static const char* digits = "0123456789abcdef";
	string temp_HEX = "";

	while (dec > 0) {
		temp_HEX = digits[dec % 16] + temp_HEX;
		dec = dec / 16;
	}

	return temp_HEX;
}
void read_watermark(int a) {
	string file = md5_file[a]+".dat";
	string HEX_watermark = "";//16進位
	string BIN_watermark = "";//2進位
	fstream fin;
	fin.open(file, ios::in);
	if (!fin) {
		throw "read watermark fail";
	}
	else {
		getline(fin, HEX_watermark);
	}
	BIN_watermark = changeHEXtoBIN(HEX_watermark);
	watermark_num = BIN_watermark.size() / (input_bit + output_bit);
	int current_ptr = 0;//利用這個類似ptr的東西來記錄現在存到哪
	for (int i = 0; i < watermark_num; i++) {
		wm[a].watermark_input.push_back(BIN_watermark.substr(current_ptr, input_bit));
		current_ptr += input_bit;
		wm[a].watermark_output.push_back(BIN_watermark.substr(current_ptr, output_bit));
		current_ptr += output_bit;
		cout << "watermark[" << i << "]: " << wm[a].watermark_input[i] << " " << wm[a].watermark_output[i] << endl;
	}
	cout << endl;
}
void output_data(int a) {
	//kiss
	string file = "ofsm.kiss";
	ofstream outfile(file, ifstream::out);
	if (!outfile.good()) {
		outfile.close();
		cout << file << " write fail!!";
		return;
	}
	string data;
	outfile << "\n";
	outfile << ".i " << input_bit << "\n";
	outfile << ".o " << output_bit << "\n";
	outfile << ".p " << transition_num << "\n";
	outfile << ".s " << state_num << "\n";
	outfile << ".r S" << start_state << "\n";

	for (int i = 0; i < origin_vecState.size(); i++) {
		map<string, NextNode>::iterator it = origin_vecState[i].begin();
		for (it; it != origin_vecState[i].end(); ++it) {
			data = it->first + " S" + to_string(i) + " S" + to_string(origin_vecState[i][it->first].next_state) + " " + origin_vecState[i][it->first].output;
			outfile << data << "\n";
		}
	}
	outfile.close();

	//env
	env = ini.size();
	string file_env = md5_file[a] + ".env";
	ofstream outfile_env(file_env, ifstream::out);
	if (!outfile_env.good()) {
		outfile_env.close();
		cout << file_env << " write fail!!";
		return;
	}
	if (env==0) {//起點在start
		outfile_env << "00000000";
	}
	else {//算得到的長度
		string getenv = changeIntToHEX(env);
		//10進改16
		for (int k = 0; k < 8; k++) {
			if (getenv.size() < 8) {
				getenv = "0" + getenv;
			}
		}
		outfile_env << getenv;
	}

	//ini
	string file_ini = md5_file[a] + ".ini";
	ofstream outfile_ini(file_ini, ifstream::out);
	if (!outfile_ini.good()) {
		outfile_ini.close();
		cout << file_ini << " write fail!!";
		return;
	}
	if (env == 0) {
		outfile_ini << " ";
	}
	else {
		for (int j = 0; j < env; j++) {
			if (j != env - 1)
				outfile_ini << ini[j] << "_";
			else
				outfile_ini << ini[j];
		}
	}
	env = 0;
	ini.clear();
	outfile_env.close();
	outfile_ini.close();
}

int main(int argc, char* argv[]) {
	//read file
	clock_t Start_t1 = clock();
	try {
		read_kiss("t3.kiss");
		if (isCSFSM()) {
			cout << "It is CSFSM!!" << endl;
		}
		for (int a = 0; a < 3; a++) {
			connect(a);
		}
	}
	catch (const char* err) {
		cout << err << endl;
	}

	cout << endl <<"Clock:" << (clock() - Start_t1) * 1.0 / CLOCKS_PER_SEC << " sec." << endl << endl;
	show_table();

	system("pause");
	return 0;
}