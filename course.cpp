#include<iostream>
#include<fstream>
#include<locale>
#include<windows.h>
#define errorMessage "Ошибка"
#define complete "Операция завершена"
using namespace std;


struct TEXT
{
	unsigned char symbol, code;
};


bool input(const string fname1,const string fname2)
{
	ifstream fin(fname1);
	if (!fin.is_open())
		return false;
	else
		fin.close();
	ofstream fout(fname2);
	if (!fout.is_open())
		return false;
	else
	{
		fout.close();
		remove(fname2.c_str());
	}
	return true;
}


void codingFileExtension(const string ffname, ofstream &ffout, unsigned int &fbitsCount)
{
	int i = 0;
	unsigned char byte;
	unsigned short int len;
	while (ffname[i] != '.')
		i++;
	len = ffname.length();
	byte = len - i - 1;
	ffout << byte;
	fbitsCount += 8;
	i++;
	while (i < len)
	{
		ffout << ffname[i];
		fbitsCount += 8;
		i++;
	}
}


bool checkingFile(const string fname)
{
	ifstream fin(fname, ios::binary);
	unsigned char sym = fin.get();
	if (fin.eof())
	{
		cout << "Файл пуст" << endl;
		fin.close();
		return false;
	}
	else
	{
		fin.close();
		return true;
	}
}


TEXT *creatingStructure(const string name, unsigned char &fstructureCount, unsigned long long int &fsymbolsCount)
{
	TEXT *fmas = new TEXT[256];
	ifstream fin(name, ios::binary);
	unsigned char sym, byte = 0;
	unsigned short int addStructureCount = 0;
	unsigned char sym2 = fin.get();
	bool checking;
	while (!fin.eof())
	{
		sym = sym2;
		fsymbolsCount++;
		checking = false;
		for (int i = 0; i < addStructureCount; i++)
			if (sym == fmas[i].symbol)
			{
				checking = true;
				break;
			}
		if (!checking)
		{
			fmas[addStructureCount].symbol = sym;
			fmas[addStructureCount].code = byte;
			byte++;
			addStructureCount++;
		}
		sym2 = fin.get();
	}
	fin.close();
	addStructureCount--;
	fstructureCount = addStructureCount;
	TEXT *fmas2 = new TEXT[fstructureCount + 1];
	for (int i = 0; i < fstructureCount + 1; i++)
	{
		fmas2[i].symbol = fmas[i].symbol;
		fmas2[i].code = fmas[i].code;
	}
	delete[]fmas;
	return fmas2;
}


unsigned char length(const unsigned short int ffstructureCount)
{
	unsigned char fcodeLength = ceil(log(ffstructureCount) / log(2.0));
	if (fcodeLength == 0)
		fcodeLength++;
	return fcodeLength;
}


unsigned char codingLastByteCount(const unsigned long long int fsymbolsCount, const unsigned char fstructureCount, unsigned int &fbitsCount)
{
	unsigned char codeLength = length(fstructureCount + 1);
	unsigned char fbyte = fbitsCount + 4 + (fstructureCount + 1) * (8 + codeLength) + codeLength * fsymbolsCount;
	fbyte %= 8;
	if (fbyte == 0)
		fbyte = 8;
	fbyte = fbyte << 4;
	fbitsCount += 4;
	return fbyte;
}


unsigned char codingStructure(const unsigned char fstructureCount, TEXT *fmas, ofstream &ffout, unsigned int &fbitsCount, unsigned char fbyte)
{
	unsigned char codeLength = length(fstructureCount + 1);
	fbyte ^= ((fstructureCount) >> 4);
	ffout << fbyte;
	fbyte = 0;
	fbyte ^= ((fstructureCount) << 4);
	fbitsCount += 8;
	for (int i = 0; i < fstructureCount + 1; i++)
	{
		fbitsCount %= 8;
		if (fbitsCount % 8 == 0)
		{
			fbyte ^= fmas[i].symbol;
			ffout << fbyte;
			fbyte = 0;
			fbitsCount += 8;
		}
		else
		{
			fbyte ^= (fmas[i].symbol >> fbitsCount % 8);
			ffout << fbyte;
			fbyte = 0;
			fbyte ^= (fmas[i].symbol << (8 - fbitsCount % 8));
			fbitsCount += 8;
		}
		if ((fbitsCount % 8 + codeLength) < 8)
		{
			fbyte ^= (fmas[i].code << (8 - fbitsCount % 8 - codeLength));
			fbitsCount += codeLength;
		}
		else
			if ((fbitsCount % 8 + codeLength) == 8)
			{
				fbyte ^= fmas[i].code;
				fbitsCount += codeLength;
				ffout << fbyte;
				fbyte = 0;
			}
			else
			{
				fbyte ^= (fmas[i].code >> (fbitsCount % 8 + codeLength - 8));
				ffout << fbyte;
				fbyte = 0;
				fbitsCount += codeLength;
				fbyte ^= (fmas[i].code << (8 - fbitsCount % 8));
			}
	}
	return fbyte;
}


void codingFile(ofstream &ffout, const string ffname, unsigned char &fbyte, TEXT *fmas, const unsigned char fstructureCount, unsigned int &fbitsCount, const unsigned long long int fsymbolsCount)
{
	ifstream fin(ffname, ios::binary);
	unsigned char addByte;
	unsigned char codeLength = length(fstructureCount + 1);
	for (int i = 0; i < fsymbolsCount; i++)
	{
		fbitsCount %= 8;
		addByte = fin.get();
		for (int j = 0; j < fstructureCount + 1; j++)
			if (addByte == fmas[j].symbol)
			{
				addByte = fmas[j].code;
				break;
			}
		if ((fbitsCount % 8 + codeLength) == 8)
		{
			fbyte ^= addByte;
			ffout << fbyte;
			fbitsCount += codeLength;
			fbyte = 0;
		}
		else
			if ((fbitsCount % 8 + codeLength) < 8)
			{
				fbyte ^= (addByte << (8 - fbitsCount % 8 - codeLength));
				fbitsCount += codeLength;
			}
			else
			{
				fbyte ^= (addByte >> (codeLength + fbitsCount % 8 - 8));
				ffout << fbyte;
				fbyte = 0;
				fbyte ^= (addByte << (16 - fbitsCount % 8 - codeLength));
				fbitsCount += codeLength;
			}
	}
	if (fbitsCount % 8 != 0)
		ffout << fbyte;
	fin.close();
}


void coding(const string fname1, const string fname2)
{
	unsigned char byte;
	unsigned long long int symbolsCount = 0;
	unsigned int bitsCount = 0;
	if (!checkingFile(fname1))
		return;
	ofstream fout(fname2, ios::binary);
	codingFileExtension(fname1, fout, bitsCount);
	unsigned char structureCount = 0;
	TEXT *mas = creatingStructure(fname1, structureCount, symbolsCount);
	byte = codingLastByteCount(symbolsCount, structureCount, bitsCount);
	byte = codingStructure(structureCount, mas, fout, bitsCount, byte);
	codingFile(fout, fname1, byte, mas, structureCount, bitsCount, symbolsCount);
	fout.close();
}


void decodingFileExtension(string &ffname2, ifstream &ffin, unsigned int &fbitsCount)
{
	unsigned int count;
	char temp;
	unsigned char byte;
	ffname2 = ffname2 + '.';
	byte = ffin.get();
	fbitsCount += 8;
	count = byte;
	for (int i = 0; i < count; i++)
	{
		byte = ffin.get();
		fbitsCount += 8;
		temp = byte;
		ffname2 = ffname2 + temp;
	}
}

unsigned char decodingLastByteCount(ifstream &ffin, unsigned int &fbitsCount, unsigned char &fLastByteCount)
{
	unsigned char ffbyte = ffin.get(), addByte = ffbyte;
	fLastByteCount = (addByte >> 4);
	fbitsCount += 4;
	return ffbyte;
}


TEXT *decodingStructure(ifstream &ffin, unsigned int &fbitsCount, unsigned char &fbyte, unsigned char &fstructureCount)
{
	fstructureCount = (fbyte << 4);
	fbyte = ffin.get();
	fstructureCount ^= (fbyte >> 4);
	fbitsCount += 8;
	TEXT *fmas = new TEXT[fstructureCount + 1];
	unsigned short int codeLength = length(fstructureCount + 1);
	for (int i = 0; i < fstructureCount + 1; i++)
	{
		fbitsCount %= 8;
		if (fbitsCount % 8 == 0)
		{
			fmas[i].symbol = fbyte;
			fbyte = ffin.get();
			fbitsCount += 8;
		}
		else
		{
			fmas[i].symbol = (fbyte << fbitsCount % 8);
			fbyte = ffin.get();
			fmas[i].symbol ^= (fbyte >> (8 - fbitsCount % 8));
			fbitsCount += 8;
		}
		if ((fbitsCount % 8 + codeLength) == 8)
		{
			fbyte <<= fbitsCount % 8;
			fbyte >>= fbitsCount % 8;
			fmas[i].code = fbyte;
			fbyte = ffin.get();
			fbitsCount += codeLength;
		}
		else
			if ((fbitsCount % 8 + codeLength) < 8)
			{
				fmas[i].code = (fbyte >> (8 - codeLength - fbitsCount % 8));
				fmas[i].code <<= (8 - codeLength);
				fmas[i].code >>= (8 - codeLength);
				fbitsCount += codeLength;
			}
			else
			{
				fmas[i].code = fbyte;
				fmas[i].code <<= fbitsCount % 8;
				fmas[i].code >>= (8 - codeLength);
				fbyte = ffin.get();
				fbitsCount += codeLength;
				fmas[i].code ^= (fbyte >> (8 - fbitsCount % 8));
			}
		if (fbitsCount % 8 == 0)
			fbitsCount = 0;
	}
	return fmas;
}

void findingSymbol(const unsigned char ffstructureCount, const unsigned char ftempByte, TEXT *ffmas, ofstream &ffout)
{
	for (int i = 0; i < ffstructureCount + 1; i++)
		if (ffmas[i].code == ftempByte)
		{
			ffout << ffmas[i].symbol;
			break;
		}
}

void decodingFile(ifstream &ffin, const string ffname, unsigned char &fbyte, unsigned char flastByteCount, unsigned int &fbitsCount, const unsigned char fstructureCount, TEXT *fmas)
{
	unsigned char codeLength = length(fstructureCount + 1);
	unsigned char lastByte, tempByte;
	ofstream fout(ffname, ios::binary);
	lastByte = ffin.get();
	while (!ffin.eof())
	{
		fbitsCount %= 8;
		if ((fbitsCount % 8 + codeLength) == 8)
		{
			fbyte <<= fbitsCount % 8;
			fbyte >>= fbitsCount % 8;
			findingSymbol(fstructureCount, fbyte, fmas, fout);
			fbitsCount += codeLength;
			fbyte = lastByte;
			lastByte = ffin.get();
		}
		else
			if ((fbitsCount % 8 + codeLength) < 8)
			{
				tempByte = fbyte << fbitsCount % 8;
				tempByte >>= (8 - codeLength);
				findingSymbol(fstructureCount, tempByte, fmas,  fout);
				fbitsCount += codeLength;
			}
			else
			{
				tempByte = fbyte << fbitsCount % 8;
				fbitsCount += codeLength;
				tempByte >>= (8 - codeLength);
				fbyte = lastByte;
				lastByte = ffin.get();
				tempByte ^= fbyte >> (8 - fbitsCount % 8);
				findingSymbol(fstructureCount, tempByte, fmas, fout);
			}
	}
	fbitsCount = 8 + fbitsCount % 8;
	flastByteCount += 8;
	while (fbitsCount < flastByteCount)
	{
		tempByte = fbyte << fbitsCount % 8;
		fbitsCount += codeLength;
		tempByte >>= (8 - codeLength);
		findingSymbol(fstructureCount, tempByte, fmas, fout);
		if (fbitsCount % 8 == 0)
			break;
	}
	fout.close();
}

void decoding(const string fname1, string fname2)
{
	unsigned char fstructureCount = 0;
	unsigned char byte, lastByteCount;
	unsigned int bitsCount = 0;
	ifstream fin(fname1, ios::binary);
	decodingFileExtension(fname2, fin, bitsCount);
	byte = decodingLastByteCount(fin, bitsCount, lastByteCount);
	TEXT *mas = decodingStructure(fin, bitsCount, byte, fstructureCount);
	decodingFile(fin, fname2, byte, lastByteCount, bitsCount, fstructureCount, mas);
	fin.close();
}

void main(int argc, char* argv[])
{
	string what;
	setlocale(LC_ALL, "russian");
	if (argc != 4)
		cout << errorMessage<<endl;
	else
	{
		what = argv[3];
		if ((input(argv[1], argv[2])) == true)
		{
			if (what == "c")
			{
				coding(argv[1], argv[2]);
				cout << complete << endl;
			}
			else
				if (what == "d")
				{
					decoding(argv[1], argv[2]);
					cout << complete << endl;
				}
				else
					cout << errorMessage << endl;
		}
		else
			cout << errorMessage << endl;
	}
}
