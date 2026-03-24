#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <time.h>
#include <stdlib.h>

// Константи для обмежень
#define MAX_OPT_TEXT 300 // Максимальна довжина тексту варіанту відповіді
#define MAX_SUBJECTS 3// 
#define MAX_OPTIONS 4 // Максимальна кількість варіантів відповіді
#define MAX_QUESTIONS 20 // Максимальна кількість питань у базі
#define MAX_DIFFICULTY 3 // Максимальний рівень складності
#define Q_WIDTH 50   // Ширина для стовпця "Питання"
#define D_WIDTH 12   // Ширина для стовпця "Складність"
#define A_WIDTH 15   // Ширина для стовпців "Відповідь 1/2/3/4"
#define R_WIDTH 14    // Ширина для стовпця "Правильна"
#define W_STAT 10 // Ширина для стовпця "Статус"

//макроси
#define isValidSubjectNum(subjectNum) ((subjectNum) >= 1 && (subjectNum) <= MAX_SUBJECTS)
#define isValidNumOptions(num) ((num) >= 1 && (num) <= MAX_OPTIONS)

typedef void (*MenuFunction)();
typedef struct {
	char name[50];
	MenuFunction func;
} MenuItem;

typedef struct complexQuestion
{
	char subject[MAX_OPT_TEXT];
	char question[MAX_OPT_TEXT];
	char answer[MAX_OPTIONS][MAX_OPT_TEXT];
	int correctAnswer;
} CQ;
int totalInDb;
FILE* database;
const char* DB_NAME = "D:\\Files\\database.bin";
char subjectNames[MAX_SUBJECTS][MAX_OPT_TEXT];

//
void addQuestion();
void searchQuestion();
void deleteQuestion();
void groupQuestionsBySubject();
void printDatabase();
void testing();
void totalTesting();
void exitProgram();

void loadSubjectsFromDB();
CQ* readQuestions();
void inputQuestion();
CQ findQuestion(char* question);
void shuffle(int* array, int n);
int askQuestion(CQ* q, int current, int total);
int runSubjectLogic(CQ* allData, const char* subjectName, int* selectedCount);
int runSubjectsTest(CQ* allData, int selectCount);

MenuItem menu[] = {
	{"Exit", exitProgram},
	{"Add Question", addQuestion},
	{"Print Database", printDatabase},
	{"Search Question", searchQuestion},
	{"Delete Question", deleteQuestion},
	{"Group by Subject", groupQuestionsBySubject},
	{"Start Testing", testing},
	{ "Start Testing Without Subjects Division", totalTesting }
};

int menuSize = sizeof(menu) / sizeof(MenuItem);

int main()
{
	srand(time(NULL));
	fopen_s(&database, DB_NAME, "rb+");
	if (database == NULL) {
		printf("Creating new DB file...\n");
		fopen_s(&database, DB_NAME, "wb+");
		inputQuestion();
	}
	else {
		fseek(database, 0, SEEK_END);
		rewind(database);
		loadSubjectsFromDB(); 
	}

	int choice;
	while (1) {
		printf("\n========== MAIN MENU ==========\n");
		for (int i = 0; i < menuSize; i++) {
			printf("%d. %s\n", i, menu[i].name);
		}
		printf("===============================\n");
		printf("Select an option: ");

		if (scanf_s("%d", &choice) != 1) {
			printf("Invalid input. Use numbers.\n");
			while (getchar() != '\n');
			continue;
		}
		rewind(stdin); 
		if (choice >= 0 && choice < menuSize) {
			menu[choice].func();
		}
		else {
			printf("Invalid choice, try again.\n");
		}
	}
}
void loadSubjectsFromDB() {
	rewind(database);
	CQ temp;
	int count = 0;

	// Очищаємо поточні назви
	for (int i = 0; i < MAX_SUBJECTS; i++) subjectNames[i][0] = '\0';

	while (fread(&temp, sizeof(CQ), 1, database) && count < MAX_SUBJECTS) {
		int found = 0;
		for (int i = 0; i < count; i++) {
			if (strcmp(subjectNames[i], temp.subject) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			strcpy_s(subjectNames[count++], MAX_OPT_TEXT, temp.subject);
		}
	}
	// Оновлюємо totalInDb про всяк випадок
	fseek(database, 0, SEEK_END);
	totalInDb = ftell(database) / sizeof(CQ);
}

void inputQuestion() {
	CQ newQuestion;
	int numSubjects = 0;
	int numQuestionsSubjectOne=0;
	int numQuestionsSubjectTwo=0;
	int numQuestionsSubjectThree=0;
	printf("Enter the list of subjects, max %d:\n", 3);
	for (int i = 0; i < 3; i++)
	{
		gets_s(subjectNames[i], MAX_OPT_TEXT);
	}
	for(int i = 0;; i++)
	{
		char answer[MAX_OPT_TEXT];
		printf("Enter subjects for question: %s - 1, %s - 2, %s - 3: ", subjectNames[0], subjectNames[1], subjectNames[2]);
		inputSubjectLabel:
		scanf_s("%d", &numSubjects);
		rewind(stdin);
		if (!isValidSubjectNum(numSubjects)) {
			printf("Invalid input, try again:");
			goto inputSubjectLabel;
		}
		if(numSubjects == 1){ numQuestionsSubjectOne ++;
		}
		else if(numSubjects == 2){
			numQuestionsSubjectTwo++;
		}
		else if(numSubjects == 3){
			numQuestionsSubjectThree++;
		}
		strcpy_s(newQuestion.subject, MAX_OPT_TEXT, subjectNames[numSubjects - 1]);
		printf("Enter the question text: ");
		gets_s(newQuestion.question, MAX_OPT_TEXT);
		for(int j = 0; j < MAX_OPTIONS; j++)
		{
			printf("Enter answer option %d: ", j + 1);
			gets_s(answer, MAX_OPT_TEXT);
			strcpy_s(newQuestion.answer[j], MAX_OPT_TEXT, answer);
		}
		printf("Enter the number of the correct answer (1-%d): ", MAX_OPTIONS);
		inputRightAnswerLAbel:
		scanf_s("%d", &newQuestion.correctAnswer);
		rewind(stdin);
		if (!isValidNumOptions(newQuestion.correctAnswer)) {
			printf("Invalid input, try again:");
			goto inputRightAnswerLAbel;
		}
		fwrite(&newQuestion, sizeof(CQ), 1, database);
		totalInDb++;
		printf("Question added successfully!\n");
		printf("Current question count for %s: %d\n", subjectNames[0], numQuestionsSubjectOne);
		printf("Current question count for %s: %d\n", subjectNames[1], numQuestionsSubjectTwo);
		printf("Current question count for %s: %d\n", subjectNames[2], numQuestionsSubjectThree);
		if (numQuestionsSubjectOne == numQuestionsSubjectTwo && numQuestionsSubjectTwo == numQuestionsSubjectThree) {
			printf("All subjects have the same number of questions. Do you want continue to input? no - N, yes - Y\n");
			endInput:
			char choice = getchar();
			if (choice == 'n' || choice == 'N') {
				break;
			}
			else if (choice == 'y' || choice == 'Y') {
				continue;
			}
			else {
				printf("Invalid choice. Write N or Y.\n");
				goto endInput;
			}
		}
	}
	printf("Input ends\n");
	return ;
}

CQ* readQuestions() {
	rewind(database);
	if (totalInDb == 0) return NULL;
	CQ* allQuestions = (CQ*)malloc(sizeof(CQ) * totalInDb);
	if (allQuestions==NULL) {
		printf("No memory");
		return NULL;
	}
	fread(allQuestions, sizeof(CQ), totalInDb, database);
	return allQuestions;
}
// Функція для групування питань за назвою дисципліни (Завдання 2.1)
void groupQuestionsBySubject() {
	if (totalInDb <= 0) return;

	CQ* allData = readQuestions();
	if (allData == NULL) return;

	// Сортування (Bubble Sort)
	for (int i = 0; i < totalInDb - 1; i++) {
		for (int j = 0; j < totalInDb - i - 1; j++) {
			if (strcmp(allData[j].subject, allData[j + 1].subject) > 0) {
				CQ temp = allData[j];
				allData[j] = allData[j + 1];
				allData[j + 1] = temp;
			}
		}
	}

	// ВАЖЛИВО: Перевідкриваємо файл у режимі "wb", щоб стерти старі дані
	fclose(database);
	fopen_s(&database, DB_NAME, "wb+");

	if (database != NULL) {
		fwrite(allData, sizeof(CQ), totalInDb, database);

		// Повертаємо режим до "rb+" для подальшої роботи
		fclose(database);
		fopen_s(&database, DB_NAME, "rb+");
	}

	printf("Questions grouped and database updated.\n");
	free(allData);
}

void addQuestion() {
	CQ addedQuestion;
	if (database==NULL) return;
	int numSubjects = 0;
	puts("Adding question to DataBase:");
	int i = 0;
	printf("Enter subjects for question: %s - 1, %s - 2, %s - 3: ", subjectNames[0], subjectNames[1], subjectNames[2]);
inputSubjectLabel:
	scanf_s("%d", &numSubjects);
	rewind(stdin);
	if (!isValidSubjectNum(numSubjects)) {
		printf("Invalid input, try again:");
		goto inputSubjectLabel;
	}
	strcpy_s(addedQuestion.subject, MAX_OPT_TEXT, subjectNames[numSubjects - 1]);
	printf("Question: ");
	gets_s(addedQuestion.question, MAX_OPT_TEXT);
	for (int i = 0; i < MAX_OPTIONS; i++) {
		printf("Answer %d: ", i + 1);
		gets_s(addedQuestion.answer[i], MAX_OPT_TEXT);
	}
	printf("Enter the number of the correct answer (1-%d): ", MAX_OPTIONS);
inputRightAnswerLAbel:
	scanf_s("%d", &addedQuestion.correctAnswer);
	rewind(stdin);
	if (!isValidNumOptions(addedQuestion.correctAnswer)) {
		printf("Invalid input, try again:");
		goto inputRightAnswerLAbel;
	}
	totalInDb++;
	fwrite(&addedQuestion, sizeof(CQ), 1, database);
	i++;
	printf("Questions Adedd\n");
}

CQ findQuestion(char* targetQuestion) {
	rewind(database);
	CQ temp;
	CQ empty = {"", "", {"", "", "", ""}, -1 }; // Структура-заглушка

	if (database == NULL) return empty;

	while (fread(&temp, sizeof(CQ), 1, database)) {
		// Використовуємо strstr для пошуку підрядка (частковий збіг)
		// Або strcmp для повної ідентичності
		if (strstr(temp.question, targetQuestion) != NULL) {
			return temp;
		}
	}
	return empty; // Якщо не знайшли
}

void searchQuestion() {
	char question[MAX_OPT_TEXT];
	puts("Search Question");
	printf("Input name of question:");
	gets_s(question, MAX_OPT_TEXT);
	rewind(stdin);
	CQ findedQuestion = findQuestion(question);
	if (findedQuestion.correctAnswer == -1) {
		printf("Error: Question not found in database.\n");
	}
	else {
		printf("\n%-15s | %-20s | %-10s | %-10s | %-10s | %-10s | %-5s\n",
			"Subject", "Question", "Ans 1", "Ans 2", "Ans 3", "Ans 4", "Right");
		printf("--------------------------------------------------------------------------------------------\n");

		printf("%-15.15s | %-20.20s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-5d\n",
			findedQuestion.subject,
			findedQuestion.question,
			findedQuestion.answer[0],
			findedQuestion.answer[1],
			findedQuestion.answer[2],
			findedQuestion.answer[3],
			findedQuestion.correctAnswer
		);
	}
}

void printDatabase() {
	CQ q;
	int i = 1;
	if (database == NULL) {
		printf("DataBase is empty or doesn`t exist\n");
		return;
	}
	rewind(database);
	printf("\n%-3s | %-15s | %-20s | %-15s | %-15s | %-15s | %-15s | %-5s\n",
		"Num", "Subject", "Question", "Ans 1", "Ans 2", "Ans 3", "Ans 4", "Right");
	printf("--------------------------------------------------------------------------------------------------------------------------------------------\n");

	while (fread(&q, sizeof(CQ), 1, database)) {
		printf("%-3d | %-15.15s | %-20.20s | %-15.15s | %-15.15s | %-15.15s | %-15.15s | %-5d\n",
			i++,
			q.subject,
			q.question,
			q.answer[0],
			q.answer[1],
			q.answer[2],
			q.answer[3],
			q.correctAnswer);
	}
	printf("--------------------------------------------------------------------------------------------------------------------------------------------\n");
}
// Before using tempArr, check if memory allocation was successful
void deleteQuestion() {
	if (totalInDb == 0) {
		printf("Database is empty!\n");
		return;
	}

	char target[MAX_OPT_TEXT];
	printf("Enter question text to delete: ");
	gets_s(target, MAX_OPT_TEXT);

	CQ* allData = readQuestions();
	if (!allData) return;

	// Закриваємо і відкриваємо в режимі "wb", щоб очистити файл
	fclose(database);
	fopen_s(&database, DB_NAME, "wb+");

	int deleted = 0;
	for (int i = 0; i < totalInDb; i++) {
		if (strstr(allData[i].question, target) == NULL) {
			fwrite(&allData[i], sizeof(CQ), 1, database);
		}
		else {
			deleted++;
		}
	}
	totalInDb -= deleted;
	free(allData);
	printf("Deleted %d questions.\n", deleted);
	fclose(database);
	fopen_s(&database, DB_NAME, "rb+");
}

int askQuestion(CQ* q, int current, int total) {
	int userAns;
	printf("\n[%d/%d] Question: %s\n", current, total, q->question);

	for (int opt = 0; opt < MAX_OPTIONS; opt++) {
		printf("  %d. %s\n", opt + 1, q->answer[opt]);
	}

	printf("Your answer (1-%d): ", MAX_OPTIONS);
	//перевірка введених даних
	while (scanf_s("%d", &userAns) != 1 || !isValidNumOptions(userAns)) {
		printf("Invalid input! Enter a number from 1 to %d: ", MAX_OPTIONS);
		rewind(stdin);
	}

	return (userAns == q->correctAnswer) ? 1 : 0;
}
void shuffle(int* array, int n) {
	for (int i = n - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		int temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

int runSubjectLogic(CQ* allData, const char* subjectName, int* selectedCount) {
	int subjectIndices[MAX_QUESTIONS];
	int countInDb = 0;
	int correctAnswers = 0;
	for (int i = 0; i < totalInDb; i++) {
		if (strcmp(allData[i].subject, subjectName) == 0) {
			subjectIndices[countInDb++] = i;
		}
	}
	if (countInDb == 0) {
		*selectedCount = 0;
		return 0;
	}
	int toSelect = (int)(countInDb * 0.75);
	if (toSelect == 0) toSelect = 1;
	*selectedCount = toSelect;
	shuffle(subjectIndices, countInDb);
	for (int i = 0; i < toSelect; i++) {
		correctAnswers += askQuestion(&allData[subjectIndices[i]], i + 1, toSelect);
	}
	return correctAnswers;
}

void testing() {
	CQ* allData = readQuestions();
	if (allData == NULL || totalInDb == 0) {
		printf("Database is empty!\n");
		return;
	}
	int correctPerSub[MAX_SUBJECTS] = { 0, 0, 0 };
	int selectedPerSub[MAX_SUBJECTS] = { 0, 0, 0 };
	int totalCorrect = 0;
	int totalSelected = 0;
	system("cls || clear");
	printf("========== STARTING TEST ==========\n");

	for (int i = 0; i < MAX_SUBJECTS; i++) {
		if (strlen(subjectNames[i]) > 0) {
			// Running test logic for each subject
			correctPerSub[i] = runSubjectLogic(allData, subjectNames[i], &selectedPerSub[i]);
			totalCorrect += correctPerSub[i];
			totalSelected += selectedPerSub[i];
		}
	}
	printf("\n================ TEST RESULTS =================\n");
	printf("%-20s | %-10s | %-10s\n", "Subject", "Correct", "Percentage");
	printf("---------------------------------------------\n");

	for (int i = 0; i < MAX_SUBJECTS; i++) {
		if (selectedPerSub[i] > 0) {
			double percent = ((double)correctPerSub[i] / selectedPerSub[i]) * 100.0;
			printf("%-20.20s | %d/%-8d | %.1f%%\n",
				subjectNames[i], correctPerSub[i], selectedPerSub[i], percent);
		}
	}

	printf("---------------------------------------------\n");
	if (totalSelected > 0) {
		printf("OVERALL RESULT: %.1f%%\n", ((double)totalCorrect / totalSelected) * 100.0);
	}
	printf("=============================================\n");

	free(allData);
	printf("\nPress Enter to return to menu...");
	rewind(stdin);
	getchar();
}

int runSubjectsTest(CQ* allData, int selectCount) {
	int* indices = (int*)malloc(sizeof(int) * totalInDb);
	if (indices == NULL) {
		printf("No enough memory");
		return -1;
	}
	for (int i = 0; i < totalInDb; i++) {
		indices[i] = i;
	}
	shuffle(indices, totalInDb);
	int correct = 0;
	system("cls || clear");
	printf("========== START TEST (%d questions) ==========\n", selectCount);
	int idx = 0;
	for (int i = 0; i < selectCount; i++) {
		CQ* q = &allData[indices[i]];
		printf("\n[%d/%d] Question: %s\n", i + 1, selectCount, q->question);
		for (int opt = 0; opt < MAX_OPTIONS; opt++) {
			printf("  %d. %s\n", opt + 1, q->answer[opt]);
		}
		int userAns;
		inputRightAnswerLabel1:
		printf("Your answer: ");
		scanf_s("%d", &userAns);
		if (!isValidNumOptions(userAns)) {
			printf("Invalid input, try again:");
			goto inputRightAnswerLabel1;
		}

		if (userAns == q->correctAnswer) {
			correct++;
		}
		printf("--------------------------------------------\n");
	}
	free(indices);
	return correct;
}
void totalTesting() {
	CQ* allData = readQuestions();
	int qPerSubject = totalInDb / MAX_SUBJECTS;
	int selectCount = (int)(totalInDb * 0.75);
	if (selectCount == 0 && totalInDb > 0) selectCount = 1;
	system("cls || clear");
	puts("========== Test ==========");
	int correct = runSubjectsTest(allData, selectCount);
	if (correct == -1) {
		printf("Error in test, try again\n");
		return;
	}
	printf("\n============================================\n");
	printf("TEST FINISHED!\n");
	printf("Total questions: %d\n", selectCount);
	printf("Correct answers: %d\n", correct);
	printf("Your score: %.1f%%\n", ((double)correct / selectCount) * 100.0);
	printf("============================================\n");
	free(allData);
	printf("\nPress Enter to return...");
	while (getchar() != '\n'); // Очистка буфера
	getchar();

}

void exitProgram() {
	printf("Saving data and exiting...\n");
	if (database) fclose(database);
	exit(0);
}