//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         泳無BUG
#include<iostream>
#include<stdlib.h>
#include<fstream>
#include<string>
#include<sstream>
#include <algorithm>
#include<math.h>
#include<vector>
#include<map>
#include<stack>
#include<time.h>
#include<cstdlib>
using namespace std;

int best_count = 0;


string ToString(int t) {
	stringstream temp;
	temp << t;
	return temp.str();
}
struct NextNode {
	string next_state;
	string output;
};

map<string, NextNode>::iterator nextIt(map<string, NextNode>::iterator it) {
	it++;
	return it;
}
string line;
int input_bit = 0;
int output_bit = 0;
int transition_num = 0;
int state_num = 0;//state 數量
string start_state = "S0";
int watermark_num;//watermark組數
string ifsm = "t2.kiss";
string ofsm = "ofsm.kiss";
string m1 ="md5_1";
string m2= "md5_2";
string m3= "md5_3";
int  maxStateName = 0;
int smallMdCost = 99999999;

struct Watermark {
	vector<string> watermark_input;//watermark input部分
	vector<string> watermark_output;//watermark output部分
	vector<vector<string> > targetState_cannotGo;//這個wm不能走到這幾個state
};
Watermark wm[3];//3個watermk的路徑
string ourStart_state = "S-1";
int env[3] = { 0 };
vector<string> ini[3];

map< string, map<string, NextNode> > origin_vecState;//存放最原始的FSM  state , <next,out>
string md5_file[3];
//connect//
struct WatermarkStyle {//存一包
	char color;
	string state;//開始的state
	int current_step;//watermark所有的位置
	int size;
	string lastState;
};
vector<WatermarkStyle>watermkGo;//存每包
								//connect//

								//<function header>
bool isCSFSM();
string isComplete(string);
string counter_add(string);
bool isINCLUDE(string, string);
void save_origin_table(string, string, string, string);
void merge();
int coloring(int);
char update_color(string, string);
void connect(int);
void read_watermark(int);
void output_data();
string find_ourStart_state(string, string, int);
string find_ourStart_state_withDC(string);
string find_state_to_makeTR(string, string);
char update_color(string, string);
void show_table();
string find_input_in_map(string, string);
bool hiding_B(int, string);//YN"N"，有隱藏的B
void recolor(int, int, string);//記錄著這一步WM與這個STATE，表示這步
							   //</function header>
int mergeCost(string, string, string, string);

void save_origin_table(string input, string state_from, string next_state, string output) {
	int state_from_int = atoi(state_from.substr(1, state_from.size() - 1).c_str());//抓state_from的string改成int
	int next_state_int = atoi(next_state.substr(1, next_state.size() - 1).c_str());//抓next_state的string改成int
	if (state_from_int > maxStateName) {
		maxStateName = state_from_int;
	}
	if (next_state_int > maxStateName) {
		maxStateName = next_state_int;
	}

	NextNode next_node;
	next_node.next_state = next_state;
	next_node.output = output;

	if (origin_vecState.find(state_from) == origin_vecState.end()) {//如果沒有找到在map中
		map<string, NextNode> insideMap;//給一個小map的空間
		origin_vecState.insert(pair<string, map<string, NextNode> >(state_from, insideMap));//小加入大map
	}
	if (origin_vecState.find(next_state) == origin_vecState.end()) {
		map<string, NextNode> insideMap;//給一個小map的空間
		origin_vecState.insert(pair<string, map<string, NextNode> >(next_state, insideMap));//小加入大map
	}

	origin_vecState[state_from].insert(pair<string, NextNode>(input, next_node));//	把小map的路加到小map
}

void read_kiss(string file_name) {
	fstream fin;
	fin.open(file_name.c_str(), ios::in);
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
					for (int a = 0; a < 3; a++)
						read_watermark(a);
					break;
				}
			}
			ss.clear();
		}
		//存好table
	}
}

bool isCSFSM() {
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		if (isComplete(itbig->first) != "Complete") {
			return false;
		}
	}
	return true;//complete
}

string isComplete(string currentState) {
	string counter = "";
	counter.resize(input_bit);//設置courter大小 = .i
	counter.assign(input_bit, '0');
	string counter_end = "";
	counter_end.resize(input_bit);//設置courter大小 = .i
	counter_end.assign(input_bit, '0'); //111 + 1 ->000 
	string counter_fullDC = "";
	counter_fullDC.resize(input_bit);
	counter_fullDC.assign(input_bit, '-');
	bool start_loop = true;
	if (origin_vecState[currentState].size() == 0) {
		return counter;
	}
	else if (origin_vecState[currentState].begin()->first == counter_fullDC) {
		return "Complete";
	}
	else {
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
}

string counter_add(string counter) {//棒棒的
									/*if (counter[counter.size() - 1] == '0') {
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
									}*/
	for (int i = counter.size() - 1; i >= 0; i--) {
		if (counter[i] == '0') {
			counter[i] = '1';
			return counter;
		}
		else {
			counter[i] = '0';
			if (i == 0) {
				return counter;
			}
		}
	}
}

bool isINCLUDE(string watermark, string FSM_input) {//DON'T CARE包含這組INPUT
	for (int i = 0; i < watermark.length(); i++) {
		if (FSM_input[i] != '-') {
			if (FSM_input[i] != watermark[i])
				return false;
		}
	}
	return true;
}

string find_input_in_map(string state, string input) {//return 包含那個INPUT的KEY
	map<string, NextNode>::iterator it;
	for (it = origin_vecState[state].begin(); it != origin_vecState[state].end(); it++) {
		if (isINCLUDE(input, it->first)) {
			return it->first;
		}
	}
	return "not_Found";
}

bool canMerge(string currentState, string targetkey, string comp1, string comp2, string output, string next_state) {
	string counter = "";
	counter.resize(input_bit);//設置courter大小 = .i
	counter.assign(input_bit, '0');
	string counter_end = "";
	counter_end.resize(input_bit);//設置courter大小 = .i
	counter_end.assign(input_bit, '0'); //111 + 1 ->000 
	bool start_loop = true;

	map<string, NextNode>::iterator it;
	while (counter != counter_end || start_loop) {//if (counter == 1111111) --> 比下一組 //start_loop 讓000=000可以進來
		if (isINCLUDE(counter, targetkey)) {//包含
			if (!isINCLUDE(counter, comp1) && !isINCLUDE(counter, comp2)) {//缺少的
				for (it = origin_vecState[currentState].begin(); it != origin_vecState[currentState].end(); it++) {
					if (isINCLUDE(counter, it->first)) {
						if (output != it->second.output || next_state != it->second.next_state) {//同一個state但是被用掉了
							return false;
						}
					}
				}
			}
			start_loop = false; // 鎖住
		}
		counter = counter_add(counter);//用下一組的counter去比較(counter+1)
	}
	return true;
}

void moreEqualTwoMerge(string currentState) {// 一個沒做到也會來count一下
	map<string, NextNode>::iterator it = origin_vecState[currentState].begin();
	map<string, NextNode>::iterator it2;
	string targetkey;
	NextNode new_value;

	bool add = true;
	while (it != origin_vecState[currentState].end()) {
		add = true;
		for (it2 = nextIt(it); it2 != origin_vecState[currentState].end(); it2++) {
			if (it->second.next_state == it2->second.next_state && it->second.output == it2->second.output) {
				targetkey.clear();

				for (int m = 0; m < it->first.size(); m++) {//2個相同
					if (it->first[m] == it2->first[m]) {
						targetkey += it->first[m];
					}
					else if (it->first[m] == '-' || it2->first[m] == '-') {//任意一個是don't care
						targetkey += '-';
					}
					else {//2個不一樣 可合成 '-'
						targetkey += '-';
					}
				}

				//可以合併or有dont 就可合
				if (canMerge(currentState, targetkey, it->first, it2->first, it->second.output, it->second.next_state)) {//能合 更新table
					new_value.output = it->second.output;
					new_value.next_state = it->second.next_state;
					string it1_first = it->first;
					string it2_first = it2->first;

					origin_vecState[currentState].erase(it);
					origin_vecState[currentState].erase(it2);
					origin_vecState[currentState].insert(pair<string, NextNode>(targetkey, new_value));
					if (isCSFSM()) {//發生Complete
						origin_vecState[currentState].erase(origin_vecState[currentState].find(targetkey));
						origin_vecState[currentState].insert(pair<string, NextNode>(it1_first, new_value));
						origin_vecState[currentState].insert(pair<string, NextNode>(it2_first, new_value));
						add = false;
						it = origin_vecState[currentState].end();/////////////////////////////////////////////////////////////// 不知道為什麼會錯...只好先這樣
						break;
					}
					else {
						transition_num--;//合少一條
						add = false;
						it = origin_vecState[currentState].begin();//重新做合
						break;
					}
				}
			}
		}
		if (add)
			it++;
	}
}

int diff(string a, string b) {
	int diffconuter = 0;
	for (int m = 0; m < b.size(); m++) {//2個相同
		if ((a[m] == '1' && b[m] == '0') || (a[m] == '0' && b[m] == '1')) {
			diffconuter++;
		}
	}
	return diffconuter;
}

void merge() {
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		map<string, NextNode>::iterator it = origin_vecState[itbig->first].begin();
		map<string, NextNode>::iterator it2;
		string newkey;
		NextNode new_value;
		int diffconuter = 0;
		int DCconuter = 0;
		int DCconuter2 = 0;
		string mergekey;
		int diff_position = 0;
		bool add = true;
		while (it != origin_vecState[itbig->first].end()) {
			add = true;
			for (it2 = nextIt(it); it2 != origin_vecState[itbig->first].end(); it2++) {
				if (it->second.next_state == it2->second.next_state && it->second.output == it2->second.output) {
					diffconuter = 0;
					DCconuter = 0;
					DCconuter2 = 0;
					newkey.clear();
					mergekey.clear();
					for (int m = 0; m < it->first.size(); m++) {//2個相同
						if (it->first[m] == it2->first[m]) {
							newkey += it->first[m];
							mergekey += it->first[m];
						}
						else if (it->first[m] == '-' && it2->first[m] != '-') {//任意一個是don't care
							newkey += '-';
							string oppo = "0";
							oppo[0] = it2->first[m];
							mergekey += counter_add(oppo);
							DCconuter++;
						}
						else if (it->first[m] != '-' && it2->first[m] == '-') {
							newkey += '-';
							string oppo = "0";
							oppo[0] = it->first[m];
							mergekey += counter_add(oppo);
							DCconuter2++;
						}
						else {//2個不一樣 可合成 '-'
							newkey += '-';
							diffconuter++;
							mergekey += it->first[m];//存好上面的 DCconuter==0
							diff_position = m;
						}

					}
					bool findCanMerge = false;
					bool finddontMerge = false;
					//任意一個1	找出一個可以把2個合起來的mergekey
					if ((DCconuter == 1 && diffconuter == 0 && DCconuter2 == 1) || (DCconuter == 1 && diffconuter == 1 && DCconuter2 == 0) || (DCconuter == 0 && diffconuter == 1 && DCconuter2 == 1)) {
						if ((DCconuter == 1 && diffconuter == 1 && DCconuter2 == 0) || (DCconuter == 0 && diffconuter == 1 && DCconuter2 == 1)) {//有diff
							if (DCconuter == 1) {
								mergekey[diff_position] = it2->first[diff_position];
							}
						}
						map<string, NextNode>::iterator it3;
						finddontMerge = true;
						for (it3 = origin_vecState[itbig->first].begin(); it3 != origin_vecState[itbig->first].end(); it3++) {
							//找到一個是包含mergekey/相同的input路
							if (isINCLUDE(mergekey, it3->first)) {
								finddontMerge = false;//被用掉了
								if (it->second.output == it3->second.output && it->second.next_state == it3->second.next_state) {
									findCanMerge = true;//存在直接用
									break;
								}
							}
							else if (diff(it3->first, mergekey) == 0) {//diffcounter > 0代表沒有互相包含一些，所以=0代表不能加進來
								findCanMerge = false;
								finddontMerge = false;
							}
						}
					}
					//只能是一個不同才可以合併or有dont 就可合
					if ((diffconuter == 1 && DCconuter == 0 && DCconuter2 == 0) || (DCconuter == 0 && diffconuter == 0) || (DCconuter2 == 0 && diffconuter == 0) || findCanMerge || finddontMerge) {//能合 更新table
						new_value.output = it->second.output;
						new_value.next_state = it->second.next_state;
						string it1_first = it->first;
						string it2_first = it2->first;

						origin_vecState[itbig->first].erase(it);
						origin_vecState[itbig->first].erase(it2);
						origin_vecState[itbig->first].insert(pair<string, NextNode>(newkey, new_value));
						if (isCSFSM()) {//發生Complete
							origin_vecState[itbig->first].erase(origin_vecState[itbig->first].find(newkey));
							origin_vecState[itbig->first].insert(pair<string, NextNode>(it1_first, new_value));
							origin_vecState[itbig->first].insert(pair<string, NextNode>(it2_first, new_value));
							add = false;
							it = origin_vecState[itbig->first].end();
							break;
						}
						else {
							transition_num--;//合少一條
							add = false;
							it = origin_vecState[itbig->first].begin();//重新做合
							break;
						}
					}
				}
			}
			if (add)
				it++;
		}
		//加一個的merge做完,做加2個以上的merge
		moreEqualTwoMerge(itbig->first);
	}
}

int coloring(int a) {//暫時
					 ////////////////////////找出最長可接的string存到watermkGo///////////////////////////////
	int cfCost = 0;//計算 Y-->R 和 R-->B 的數量 (conflict cost
	int i = 0;
	watermkGo.clear();
	while (i < wm[a].watermark_input.size()) {//走完wm
		vector<WatermarkStyle> roads;//存下每個wm_input,output多少組可走的路

		map<string, map<string, NextNode> >::iterator itbig;
		for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {//每個state去找
			map<string, NextNode>::iterator it;
			for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
				if (isINCLUDE(wm[a].watermark_input[i], it->first) && isINCLUDE(wm[a].watermark_output[i], it->second.output))//originFSM中找到wm的input一樣的key(input)且output也對
				{
					//0825新增 為了解決YYN"N"
					bool canUse_Y = true;
					if (wm[a].targetState_cannotGo[i].size() != 0) {
						vector<string>::const_iterator result = find(wm[a].targetState_cannotGo[i].begin(), wm[a].targetState_cannotGo[i].end(), origin_vecState[itbig->first][it->first].next_state);
						if (result != wm[a].targetState_cannotGo[i].end()) {
							canUse_Y = false;
						}
					}
					if (canUse_Y) {
						WatermarkStyle WStemp;
						WStemp.color = 'Y';
						WStemp.current_step = i;//目前wm pos
						WStemp.state = itbig->first;
						WStemp.lastState = it->second.next_state;
						WStemp.size = 1;
						roads.push_back(WStemp);//存下這條可走的路
					}
				}
			}
		}
		//當沒有一條可走的路
		if (roads.size() == 0) {//白色and藍色and紅色
			int dontcare = 0;//用來記錄有DONTCARE的STATE數
			map<string, map<string, NextNode> >::iterator itbig;
			for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {//每個state去看
				map<string, NextNode>::iterator it;
				if (origin_vecState[itbig->first].size() == 0) {
					dontcare++;
				}
				else {
					bool conflict = false;
					for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
						if (isINCLUDE(wm[a].watermark_input[i], it->first)) {//這個state有這個input(CONFLICT
							conflict = true;
							break;
						}
					}
					if (!conflict) {
						dontcare++;
					}
				}
			}

			WatermarkStyle WStemp;
			if (dontcare == 0) //藍色
				WStemp.color = 'B';
			else if (dontcare == state_num)//白色
				WStemp.color = 'W';
			else //無色
				WStemp.color = 'N';
			WStemp.current_step = i;
			WStemp.state = "S-1";
			WStemp.lastState = "S-1";
			WStemp.size = 1;
			watermkGo.push_back(WStemp);//放入一個東西
			i++;
		}
		else {//一包的ck
			WatermarkStyle WSmin;//初始值
			WSmin.color = 'Y';
			WSmin.current_step = i;
			WSmin.state = "S-1";
			WSmin.lastState = "S-1";
			WSmin.size = 0;
			watermkGo.push_back(WSmin);//先放最小值,才可以比較出roads的大小(找出最長string)

									   //srand(time(NULL));
			int countbreak = 0;
			for (int j = 0; j < roads.size(); j++) {//試所有能走的路
				string currentState = roads[j].lastState;//現在的state要是next_state的index
				int i_ptr = i;//存檔點

				while (1) {
					i_ptr++;//下一個wm拿去比
					if (i_ptr < wm[a].watermark_input.size()) {//防爆(超出範圍)
															   //input有找到
						map<string, NextNode>::iterator it;
						bool findInput = false;
						/*if*/	for (it = origin_vecState[currentState].begin(); it != origin_vecState[currentState].end(); it++) {
							if (isINCLUDE(wm[a].watermark_input[i_ptr], it->first) && isINCLUDE(wm[a].watermark_output[i_ptr], it->second.output)) {//能走下去的路
								currentState = it->second.next_state;
								roads[j].size++;//路的長度++
								roads[j].lastState = currentState;
								findInput = true;
								break;
							}
						}
						/*else*/if (!findInput) {//斷開
							map<string, NextNode>::iterator it;
							bool isConflict = false;
							//有input但output不一樣(conflict)不能存
							for (it = origin_vecState[currentState].begin(); it != origin_vecState[currentState].end(); it++) {
								if (isINCLUDE(wm[a].watermark_input[i_ptr], it->first) && !isINCLUDE(wm[a].watermark_output[i_ptr], it->second.output)) {
									countbreak++;
									isConflict = true;
									break;
								}
							}
							if (isConflict) {//跳出while(1)不要存    //如果全conflict跳到if (countbreak == roads.size()) 
								break;
							}
							else {//(沒conflict)存到watermkGO
								if (watermkGo[watermkGo.size() - 1].size < roads[j].size) {
									watermkGo.pop_back();
									watermkGo.push_back(roads[j]);
								}
								else if (watermkGo[watermkGo.size() - 1].size == roads[j].size && rand() % (rand() % 2 + 2) != 1) {/////////////////////////////////
									watermkGo.pop_back();
									watermkGo.push_back(roads[j]);

								}
								break;
							}
						}
					}
					else {//是最後一個 不要比
						watermkGo.pop_back();
						watermkGo.push_back(roads[j]);
						break;
					}
				}
				if (countbreak == roads.size()) {//全conflict就是紅色
					cfCost++;
					roads[j].color = update_color(wm[a].watermark_input[roads[j].current_step], wm[a].watermark_output[roads[j].current_step]);//紅色判斷是否為藍色
					if (roads[j].color == 'B')
						cfCost++;
					roads[j].state = "S-1";
					roads[j].lastState = "S-1";
					roads[j].size = 1;
					watermkGo.pop_back();
					watermkGo.push_back(roads[j]);
				}
			}
			i = watermkGo[watermkGo.size() - 1].size + i;//i到包尾的下一個
		}
		//cout << "watermark[" << watermkGo[watermkGo.size() - 1].current_step << "]: " << wm[a].watermark_input[watermkGo[watermkGo.size() - 1].current_step] << " " << wm[a].watermark_output[watermkGo[watermkGo.size() - 1].current_step] << endl;
		//cout << watermkGo[watermkGo.size() - 1].color << " Step:" << watermkGo[watermkGo.size() - 1].current_step << " Size:" << watermkGo[watermkGo.size() - 1].size << " " << watermkGo[watermkGo.size() - 1].state << " last: " << watermkGo[watermkGo.size() - 1].lastState << endl;
	}
	//cout << endl;
	return cfCost;
	////////////////////////找出最長可接的string存到watermkGo///////////////////////////////
}

char update_color(string intput, string output) {//R-->B
	int dontcare = 0;//用來記錄有DONTCARE的STATE數
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {//每個state去看
		map<string, NextNode>::iterator it;
		if (origin_vecState[itbig->first].size() == 0) {
			dontcare++;
		}
		else {
			bool conflict = false;
			for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
				if (isINCLUDE(intput, it->first)) {//這個state有這個input(CONFLICT
					conflict = true;
					break;
				}
			}
			if (!conflict) {
				dontcare++;
			}
		}
	}
	char color = 'R';
	if (dontcare == 0) //藍色
		color = 'B';
	else //紅色
		color = 'R';
	return color;
}

void recolor(int a, int wmCurrent_step, string nextState) {
	wm[a].targetState_cannotGo[wmCurrent_step].push_back(nextState);
	coloring(a);
}

int mergeCost(string currentState,string nextState ,string input ,string output) {
	int canMerge = 1;
	bool outputSame = false;
	map<string, NextNode>::iterator it;
	for (it = origin_vecState[currentState].begin(); it != origin_vecState[currentState].end();it++) {
		if (it->first != input && it->second.output == output && it->second.next_state == nextState) {//把自己濾掉//不等於自己 //output相等
			outputSame = true;
			if (diff(it->first, input) == 1) {//看看哪個nextState可以merge更多
				canMerge++;
			}
		}
	}
	if (canMerge != 0) return canMerge;
	if (outputSame)return 1;
	return 0;
}

void connect(int a) {//開連
					 //srand(time(NULL));
	string our_currentState = start_state;
	string our_nextState = start_state;

	//<總之先找第一個STATE在哪
	if (watermkGo[0].color == 'Y') {//Y開頭
		if (watermkGo.size() == 1) {//WM已經都在FSM裡面了(只剩一包黃色)
			ourStart_state = find_ourStart_state(start_state, watermkGo[0].state, a);//走到那個STATE

			if (a == 2) {
				merge();
				if (transition_num + state_num <= 110) {
					cout << " " << "find transition:" << transition_num << " state:" <<state_num;
					best_count++;
				}
				if ((smallMdCost > (transition_num + state_num))) {//跑完再看
					smallMdCost = transition_num + state_num;
					output_data();
				}
			}
			return;
		}
		else {//生出下一個TR
			our_currentState = watermkGo[0].lastState;
			string newRoad = wm[a].watermark_input[watermkGo[1].current_step];
			string newRoad_output = wm[a].watermark_output[watermkGo[1].current_step];
			if (watermkGo.size() == 2) {//Y開頭，且剩兩包
				if (watermkGo[1].color == 'Y' && watermkGo[1].size > 1) { //YY  下一個也是Y且size > 1就要接到第2包Y的第2個STATE(完成
					our_nextState = origin_vecState[watermkGo[1].state][find_input_in_map(watermkGo[1].state, newRoad)].next_state;//用newRoad找到那條路
					save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
					if (hiding_B(a, our_currentState)) {
						origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
						recolor(a, watermkGo[1].current_step, our_nextState);//將這個STATE記錄起來，表示這一步waterMK不能走到這
					}
					else {
						our_currentState = our_nextState;
						transition_num++;
					}
				}
				else {//YR or YW or YY(這個第二個Y的size是1 所以做YR的事)  (不會有YB)//因為只剩一組了，隨便連到一個STATE就好(完成
					map<string, map<string, NextNode> >::iterator itbig;
					for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
						if (rand() % (rand() % 2 + 2) != 1) {
							our_nextState = itbig->first;
						}
					}

					save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
					if (hiding_B(a, our_currentState)) {//發生Complete
						origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
						our_nextState = "S" + ToString(maxStateName + 1);
						maxStateName++;
						save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
						state_num++;
					}
					our_currentState = our_nextState;
					transition_num++;
				}
			}
			else {//Y開頭，三包以上
				if (watermkGo[1].color == 'Y' && watermkGo[1].size > 1) { //YYX  //下一包也是Y就連到那包的第2個state(完成
					our_nextState = origin_vecState[watermkGo[1].state][find_input_in_map(watermkGo[1].state, newRoad)].next_state;//用newRoad找到那條路
					save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
					if (hiding_B(a, our_currentState)) {
						origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
						recolor(a, watermkGo[1].current_step, our_nextState);//將這個STATE記錄起來，表示這一步waterMK不能走到這
					}
					else {
						our_currentState = our_nextState;
						transition_num++;
					}
				}
				else {//YRX or YWX or YNX //YYY(這裡的第二個Y大小為1)
					if (watermkGo[2].color == 'Y') { //YRY or YWY or YNY or YYY //目標STATE為第二個Y的起始STATE
						our_nextState = watermkGo[2].state;

						save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
						if (hiding_B(a, our_currentState)) {
							origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
							recolor(a, watermkGo[2].current_step, origin_vecState[our_nextState].find(find_input_in_map(our_nextState,wm[a].watermark_input[watermkGo[2].current_step]))->second.next_state);//將這個STATE記錄起來，表示這一步waterMK不能走到這
						}
						else {
							our_currentState = our_nextState;
							transition_num++;
						}
					}
					else if (watermkGo[2].color == 'N' || watermkGo[2].color == 'W' || watermkGo[2].color == 'R') {//YRR or YWR or YRW or YWW  //隨便連到一個不會conflict的STATE就好(完成，待優化，因為有可能發生新增TR到一個STATE結果造成B出現
						bool noStateHaveDC = true;
						map<string, map<string, NextNode> >::iterator itbig;
						map<string, int> cfCosts;
						for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
							map<string, NextNode>::iterator it;
							bool conflict = false;
							if (origin_vecState[itbig->first].size() != 0) {
								for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
									if (isINCLUDE(wm[a].watermark_input[watermkGo[2].current_step], it->first)) {//有conflict
										conflict = true;
										break;
									}
								}
							}
							if (conflict == false) {//找到不會conflict的state
								our_nextState = itbig->first;
								save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
								if (!hiding_B(a, our_currentState)) {//歐K
									cfCosts.insert(pair<string, int>(itbig->first, coloring(a)));
									noStateHaveDC = false;
								}
								origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
								coloring(a);
							}
						}
						if (noStateHaveDC) {//有隱藏的YN"B"   //這裡要做的事跟YRB一樣
							our_nextState = "S" + ToString(maxStateName + 1);
							maxStateName++;
							save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
							our_currentState = our_nextState;
							transition_num++;
							state_num++;
						}
						else {
							int min_cfCost = 999999;
							int max_mergeCost = -1;
							string best_state;
							for (map<string, int>::iterator it_cfCosts = cfCosts.begin(); it_cfCosts != cfCosts.end(); it_cfCosts++) {
								if (min_cfCost > it_cfCosts->second) {//存cfCost小的那個
									min_cfCost = it_cfCosts->second;
									max_mergeCost = mergeCost(our_currentState, our_nextState, newRoad, newRoad_output);
									best_state = it_cfCosts->first;
								}
								else if (min_cfCost == it_cfCosts->second) {//cfCost一樣大的話，先判斷mergeCost
									if (max_mergeCost < mergeCost(our_currentState, our_nextState, newRoad, newRoad_output)) {//存mergeCost大的那個
										min_cfCost = it_cfCosts->second;
										max_mergeCost = mergeCost(our_currentState, our_nextState, newRoad, newRoad_output);
										best_state = it_cfCosts->first;
									}
									else if (max_mergeCost == mergeCost(our_currentState, our_nextState, newRoad, newRoad_output)) {//mergeCost一樣的話rand
										if (rand() % (rand() % 2 + 2) != 1) {
											min_cfCost = it_cfCosts->second;
											max_mergeCost = mergeCost(our_currentState, our_nextState, newRoad, newRoad_output);
											best_state = it_cfCosts->first;
										}
									}
								}
							}
							save_origin_table(newRoad, our_currentState, best_state, newRoad_output);
							our_currentState = best_state;
							transition_num++;
							cfCosts.clear();
						}
					}
					else {//YRB or YWB   //生一個STATE，再用那個RorB走到那個STATE
						our_nextState = "S" + ToString(maxStateName + 1);//生的state要比state數大1
						maxStateName++;
						//map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
						//origin_vecState.push_back(mapNextState);
						save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
						our_currentState = our_nextState;
						transition_num++;
						state_num++;
					}
				}
			}
		}
	}
	else if (watermkGo[0].color == 'B') {//B開頭   //先生出一個STATE 再去新增藍色TRANSITION，讓它變成紅色再reColor(完成   //待改(隨機選起始STATE
		ourStart_state = find_ourStart_state_withDC(start_state);//找到一個有dont care的node
		string newRoad = isComplete(ourStart_state);

		//從我可走的點出去 = ourStart_state
		our_currentState = ourStart_state;
		our_nextState = "S" + ToString(maxStateName + 1);
		maxStateName++;
		//map<string, NextNode> mapNextState;//以input為key存放value(nextState & output)
		//origin_vecState.push_back(mapNextState);
		save_origin_table(newRoad, our_currentState, our_nextState, wm[a].watermark_output[0]);
		our_currentState = our_nextState;
		transition_num++;
		state_num++;
		ourStart_state = origin_vecState.size() - 1;
	}
	else {//R or W or N id開頭(未開始 //////////////////待改(隨機選起始STATE
		ini[a].clear();
		ourStart_state = find_state_to_makeTR(start_state, wm[a].watermark_input[0]);//找到一個可以新增第一個WM的STATE
		our_currentState = ourStart_state;
		string newRoad = wm[a].watermark_input[watermkGo[0].current_step];
		string newRoad_output = wm[a].watermark_output[watermkGo[0].current_step];
		if (watermkGo.size() > 1) {//正常情況(2包以上)
			if (watermkGo[1].color == 'B') {//RB  //這個R用來新增TR連到新增的STATE
				our_nextState = "S" + ToString(maxStateName + 1);
				maxStateName++;
				save_origin_table(newRoad, our_currentState, our_nextState, wm[a].watermark_output[0]);
				our_currentState = our_nextState;
				transition_num++;
				state_num++;
				ourStart_state = origin_vecState.size() - 1;
			}
			else if (watermkGo[1].color == 'Y') {//RY //這個R用來新增TR連到那包Y
				our_nextState = watermkGo[1].state;
				save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
				if (hiding_B(a, our_currentState)) {
					origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
					recolor(a, watermkGo[1].current_step, our_nextState);//將這個STATE記錄起來，表示這一步waterMK不能走到這
				}
				else {
					our_currentState = our_nextState;
					transition_num++;
				}
			}
			else {//RR  //隨便找個不會CONFLICT的STATE插上去
				map<string, map<string, NextNode> >::iterator itbig;
				map<string, int> cfCosts;
				bool noStateHaveDC = true;
				for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
					map<string, NextNode>::iterator it;
					bool conflict = false;
					if (origin_vecState[itbig->first].size() != 0) {
						for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
							if (isINCLUDE(wm[a].watermark_input[watermkGo[1].current_step], it->first)) {//有conflict
								conflict = true;
								break;
							}
						}
					}
					if (conflict == false) {//找到不會conflict的state
						our_nextState = itbig->first;
						save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
						if (!hiding_B(a, our_currentState)) {//歐K
							cfCosts.insert(pair<string, int>(itbig->first, coloring(a)));
							noStateHaveDC = false;
						}
						origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
						coloring(a);
					}
				}
				if (noStateHaveDC) {//有隱藏的R"B"   //這裡要做的事跟RB一樣
					our_nextState = "S" + ToString(maxStateName + 1);
					maxStateName++;

					save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
					our_currentState = our_nextState;
					transition_num++;
					state_num++;
				}
				else {
					int min_cfCost = 999999;
					int max_mergeCost = -1;
					string best_state;
					for (map<string, int>::iterator it_cfCosts = cfCosts.begin(); it_cfCosts != cfCosts.end(); it_cfCosts++) {
						if (min_cfCost > it_cfCosts->second) {//存cfCost小的那個
							min_cfCost = it_cfCosts->second;
							best_state = it_cfCosts->first;
						}
						else if (min_cfCost == it_cfCosts->second) {//cfCost一樣大的話，先判斷mergeCost
							if (max_mergeCost < mergeCost(our_currentState, our_nextState, newRoad, newRoad_output)) {//存mergeCost大的那個
								min_cfCost = it_cfCosts->second;
								max_mergeCost = mergeCost(our_currentState, our_nextState, newRoad, newRoad_output);
								best_state = it_cfCosts->first;
							}
							else if (max_mergeCost == mergeCost(our_currentState, our_nextState, newRoad, newRoad_output)) {//mergeCost一樣的話rand
								if (rand() % (rand() % 2 + 2) != 1) {
									min_cfCost = it_cfCosts->second;
									max_mergeCost = mergeCost(our_currentState, our_nextState, newRoad, newRoad_output);
									best_state = it_cfCosts->first;
								}
							}
						}
					}
					save_origin_table(newRoad, our_currentState, best_state, newRoad_output);
					our_currentState = best_state;
					transition_num++;
					cfCosts.clear();
				}
			}
		}
		else if (watermkGo.size() == 1) {//R
			our_nextState = start_state;
			save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
			if (isCSFSM() && a < 2) {//發生Complete
				origin_vecState[our_currentState].erase(origin_vecState[our_currentState].find(newRoad));
				our_nextState = "S" + ToString(maxStateName + 1);
				maxStateName++;
				save_origin_table(newRoad, our_currentState, our_nextState, newRoad_output);
				state_num++;
			}
			transition_num++;
		}
	}
	//開連!!!
	//cout << endl;
	coloring(a);
	connect(a);
}

bool hiding_B(int a, string currentState) {
	int i;
	if (watermkGo[0].color == 'Y') {//YXXXXXX，總之延著走
		i = watermkGo[1].current_step;
	}
	else {//RXXXXX
		i = 0;
	}
	map<string, NextNode>::iterator it;
	for (i; i < wm[a].watermark_input.size(); i++) {//從第一包Y的下一個wm開始走到底，也就是剛剛新增的那一條
		bool haveDontcare = true;//如果沒路可走了又沒有conflict，就回傳false，表示沒有hiding_B
		for (it = origin_vecState[currentState].begin(); it != origin_vecState[currentState].end(); it++) {
			if (isINCLUDE(wm[a].watermark_input[i], it->first)) {//input一樣
				if (wm[a].watermark_output[i] != it->second.output) {//output不一樣，conflict
					return true;
				}
				else {//output一樣，走
					currentState = it->second.next_state;
					haveDontcare = false;
					break;
				}
			}
		}
		if (haveDontcare) {
			return false;
		}
	}
	//如果走到這，代表全部吃完，所以要檢查是不是CSFSM
	if (a < 2) {
		if (isCSFSM()) {//發生Complete
			return true;
		}
	}
	return false;
}

string find_ourStart_state(string currentState, string targetState, int a) {//找到目標STATE
	stack<string> sk;
	map<string, int> it_counter;//計算用過的路的數量
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		it_counter.insert(pair<string, int>(itbig->first, 0));
	}
	map<string, NextNode>::iterator it = origin_vecState[currentState].begin();
	sk.push(currentState);

	while (1) {//small map
		if (it_counter[currentState] != 0 || (it == origin_vecState[currentState].end() && origin_vecState[currentState].size() != 0)) {//如果current state有走過或已經走到底，就要pop回去
																																		//pop
			if (!sk.empty())
				sk.pop();
			currentState = sk.top();
			ini[a].pop_back();

			//it指向上一個STATE的下一條路
			it = origin_vecState[currentState].begin();
			for (int i = 0; i < it_counter[currentState]; i++)
				it++;
		}
		if (it != origin_vecState[currentState].end() || origin_vecState[currentState].size() == 0) {
			if (currentState == targetState) {//找到目標STATE
				return currentState;//state
			}
			else {//目前STATE不是目標STATE
				if (origin_vecState[currentState].size() == 0) {
					it_counter[currentState]++;
				}
				else {
					it_counter[currentState]++;//這個STATE的it counter++
					currentState = it->second.next_state;
					sk.push(currentState);
					//---11---  -->  00011000 
					string ini_tmp = it->first;
					for (int g = 0; g<ini_tmp.size(); g++) {
						if (ini_tmp[g] == '-') {
							ini_tmp[g] = '0';
						}
					}
					ini[a].push_back(ini_tmp);
					it = origin_vecState[currentState].begin();
				}
			}
		}
	}
}

string find_ourStart_state_withDC(string currentState) {//找到最近的一個有dont care的state
	stack<string> sk;
	map<string, int> it_counter;//計算用過的路的數量
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		it_counter.insert(pair<string, int>(itbig->first, 0));
	}
	map<string, NextNode>::iterator it = origin_vecState[currentState].begin();
	sk.push(currentState);

	while (1) {//map

		if (it_counter[currentState] != 0 || (it == origin_vecState[currentState].end() && origin_vecState[currentState].size() != 0)) {//如果current state有走過或已經走到底，就要pop回去
																																		//pop
			if (!sk.empty())
				sk.pop();
			currentState = sk.top();

			//it指向上一個STATE的下一條路
			it = origin_vecState[currentState].begin();
			for (int i = 0; i < it_counter[currentState]; i++)
				it++;
		}
		if (it != origin_vecState[currentState].end() || origin_vecState[currentState].size() == 0) {
			if (isComplete(currentState) != "Complete") {//有dont care
				return currentState;//state
			}
			else {//complete
				it_counter[currentState]++;//這個STATE的it counter++
				currentState = it->second.next_state;
				sk.push(currentState);
				it = origin_vecState[currentState].begin();
			}
		}
	}
}

string find_state_to_makeTR(string currentState, string input) {//隨便找一個可以生transition的state
	map<string, bool> yelloEnd;//計算用過的路的數量
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		yelloEnd.insert(pair<string, bool>(itbig->first, false));
	}
	for (int i = 0; i < watermkGo.size(); i++) {//把黃包的結尾state設成true
		if (watermkGo[i].color == 'Y') {
			yelloEnd[watermkGo[i].lastState] = true;
		}
	}

	//找一個可以生transition的state
	map<string, NextNode>::iterator it;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {//先看不是黃包結尾的
		if (!yelloEnd[itbig->first]) {
			bool have_dontcare = true;
			for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
				if (isINCLUDE(input, it->first)) {
					have_dontcare = false;
					break;
				}
			}
			if (have_dontcare) {
				return itbig->first;
			}
		}
	}
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {//再看是黃包結尾的
		if (yelloEnd[itbig->first]) {
			bool have_dontcare = true;
			for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
				if (isINCLUDE(input, it->first)) {
					have_dontcare = false;
					break;
				}
			}
			if (have_dontcare) {
				return itbig->first;
			}
		}
	}
	return "S-1";
}

void show_table() {
	map<string, NextNode>::iterator it;
	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		cout << itbig->first << ": ";
		for (it = origin_vecState[itbig->first].begin(); it != origin_vecState[itbig->first].end(); it++) {
			cout << it->first << " ";
		}
		cout << endl;
	}
	cout << endl << endl;
}

string changeHEXtoBIN(string HEX_watermark) {
	string temp_BIN_watermark = "";
	//transform(HEX_watermark.begin(), HEX_watermark.end(), HEX_watermark.begin(), tolower);//大換小英文字
	for (int i = 0; i < HEX_watermark.size(); i++)
	{
		int num;
		if (HEX_watermark[i] >= 'a' && HEX_watermark[i] <= 'f')
			num = HEX_watermark[i] - 'a' + 10;//16->10進位
		else
			num = HEX_watermark[i] - '0';

		string bin;
		while (num > 0) {//2進
			bin = ToString(num % 2) + bin;//把最後的存到前面去
			num /= 2;
		}

		while (bin.size() < 4) { //補0 bit
			bin = "0" + bin;
		}
		temp_BIN_watermark += bin;
	}

	//cout << temp_BIN_watermark << endl;

	//若128除以(input_bit + output_bit)有餘數,補0
	int remainder = 128 % (input_bit + output_bit);
	if (remainder != 0) {
		int lack = input_bit + output_bit - remainder;//缺多少bit
		for (int i = 0; i < lack; i++) {
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
	string file = md5_file[a] + ".dat";
	string HEX_watermark = "";//16進位
	string BIN_watermark = "";//2進位
	fstream fin;
	fin.open(file.c_str(), ios::in);
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
		//cout << "watermark[" << i << "]: " << wm[a].watermark_input[i] << " " << wm[a].watermark_output[i] << endl;

		vector<string> cannotGo;
		wm[a].targetState_cannotGo.push_back(cannotGo);
	}
	//cout << endl;
}
void output_data() {
	//kiss
	string file = ofsm;
	ofstream outfile(file.c_str(), ifstream::out);
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
	outfile << ".r " << start_state << "\n";

	map<string, map<string, NextNode> >::iterator itbig;
	for (itbig = origin_vecState.begin(); itbig != origin_vecState.end(); itbig++) {
		map<string, NextNode>::iterator it = origin_vecState[itbig->first].begin();
		for (it; it != origin_vecState[itbig->first].end(); ++it) {
			data = it->first + " " + itbig->first + " " + origin_vecState[itbig->first][it->first].next_state + " " + origin_vecState[itbig->first][it->first].output;
			outfile << data << "\n";
		}
	}

	outfile << ".e" << endl;
	outfile.close();

	for (int a = 0; a < 3; a++) {
		//env
		env[a] = ini[a].size();
		string file_env = md5_file[a] + ".env";
		ofstream outfile_env(file_env.c_str(), ifstream::out);
		if (!outfile_env.good()) {
			outfile_env.close();
			cout << file_env << " write fail!!";
			return;
		}
		if (env[a] == 0) {//起點在start
			outfile_env << "00000000";
		}
		else {//算得到的長度
			string getenv = changeIntToHEX(env[a]);
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
		ofstream outfile_ini(file_ini.c_str(), ifstream::out);
		if (!outfile_ini.good()) {
			outfile_ini.close();
			cout << file_ini << " write fail!!";
			return;
		}
		if (env[a] == 0) {
			outfile_ini << " ";
		}
		else {
			for (int j = 0; j < env[a]; j++) {
				if (j != env[a] - 1)
					outfile_ini << ini[a][j] << "_";
				else
					outfile_ini << ini[a][j];
			}
		}
		//env[a] = 0;
		//ini.clear();
		outfile_env.close();
		outfile_ini.close();
	}
	//show_table();
	//cout << "order: " << env[0] << ": " << md5_file[0] << " " << env[1] << ": " << md5_file[1] << " " << env[2] << ": " << md5_file[2] << "///////////////////////////////////////////////////////////////" << endl;
}
int main(int argc, char* argv[]) {
	//read file
	//clock_t Start_t1 = clock();
	srand(time(NULL));
	try {
		int mc = 0;
		for (int a = 0; a < argc; a++) {
			string tmp = argv[a];
			if (tmp == "-i") {
				ifsm = argv[a + 1];
			}
			else if (tmp == "-o") {
				ofsm = argv[a + 1];
			}
			else if (tmp == "-m") {
				mc++;
				if (mc == 1) {
					m1 = argv[a + 1];
					m1 = m1.substr(0, m1.length() - 4);
				}
				else if (mc == 2) {
					m2 = argv[a + 1];
					m2 = m2.substr(0, m2.length() - 4);
				}
				else if (mc == 3) {
					m3 = argv[a + 1];
					m3 = m3.substr(0, m3.length() - 4);
				}
			}
		}
		string md5_file_6[6][3] = { { m1,m2,m3 },{ m1,m3,m2 },{ m2,m1,m3 },{ m2,m3,m1 },{ m3,m1,m2 },{ m3,m2,m1 } };
		for (int i = 0; i < 3; i++) {
			md5_file[i] = md5_file_6[0][i];
		}
		read_kiss(ifsm);
		if (isCSFSM()) {
			cout << "It is CSFSM!!" << endl;
		}
		else {
			bool first = true;
			//存好 origin_table後 合併
			for (int t = 0; t < 100; t++) {
				cout << endl << t << ":";
				for (int md_times = 0; md_times < 6; md_times++) {
					cout << "  " << md_times;
					for (int i = 0; i < 3; i++) {
						md5_file[i] = md5_file_6[md_times][i];
					}

					if (!first)
						read_kiss(ifsm);
					first = false;
					merge();
					for (int a = 0; a < 3; a++) {
						coloring(a);
						connect(a);
						//cout << endl;
						//cout << "--------------------------------------------------------------------" << endl << endl;
					}
					//得出md5排列的cost,好的就存好
					//clean
					for (int i = 0; i<3; i++) {
						ini[i].clear();
						env[i] = 0;
						wm[i].watermark_input.clear();
						wm[i].watermark_output.clear();
						wm[i].targetState_cannotGo.clear();
					}
					origin_vecState.clear();
					maxStateName = 0;
				}
			}
		}
	}
	catch (const char* err) {
		cout << err << endl;
	}
	cout << endl;
	cout << "best count: " << best_count << endl;
	//cout << endl << "Clock:" << (clock() - Start_t1) * 1.0 / CLOCKS_PER_SEC << " sec." << endl << endl;
	//system("pause");
	//getchar();
	return 0;
}

