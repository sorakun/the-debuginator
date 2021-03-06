#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct UnitTestData
{
	char errors[256][256];
	unsigned error_index;
	unsigned num_tests;

	bool simplebool_target;

	char stringtest[256];
} UnitTestData;

static UnitTestData g_testdata;

static void unittest_debuginator_assert(bool test) {
	++g_testdata.num_tests;
	if (g_testdata.error_index == 256) {
		assert(false);
	}
	if (!test) {
		memcpy(g_testdata.errors[g_testdata.error_index++], "LOL", 4);
	}
	//DebugBreak();
}

#define DEBUGINATOR_assert unittest_debuginator_assert
#define ASSERT unittest_debuginator_assert

#define DEBUGINATOR_debug_print printf
#define DEBUGINATOR_IMPLEMENTATION

#include "../../the_debuginator.h"


#pragma warning(suppress: 4100) // Unreferenced param
void draw_text(const char* text, DebuginatorVector2* position, DebuginatorColor* color, DebuginatorFont* font, void* userdata) {
}

#pragma warning(suppress: 4100) // Unreferenced param
void draw_rect(DebuginatorVector2* position, DebuginatorVector2* size, DebuginatorColor* color, void* userdata) {
}

#pragma warning(suppress: 4100) // Unreferenced param
const char* word_wrap(const char* text, DebuginatorFont font, float max_width, char* buffer, int buffer_size, void* userdata) {
	return NULL;
}

static void unittest_on_item_changed_stringtest(DebuginatorItem* item, void* value, const char* value_title, void* app_userdata) {
	(void)value_title;
	(void)app_userdata;
	const char** string_ptr = (const char**)value;
	UnitTestData* callback_data = (UnitTestData*)item->user_data; // same as &g_testdata

#ifdef __cplusplus
	strncpy_s(callback_data->stringtest, *string_ptr, strlen(*string_ptr));
#else
	strncpy_s(callback_data->stringtest, sizeof(callback_data->stringtest), *string_ptr, strlen(*string_ptr));
#endif
}

static void unittest_debug_menu_setup(TheDebuginator* debuginator) {
	debuginator_create_bool_item(debuginator, "SimpleBool 1", "Change a bool.", &g_testdata.simplebool_target);
	debuginator_create_bool_item(debuginator, "Folder/SimpleBool 2", "Change a bool.", &g_testdata.simplebool_target);
	debuginator_create_bool_item(debuginator, "Folder/SimpleBool 3", "Change a bool.", &g_testdata.simplebool_target);
	debuginator_create_bool_item(debuginator, "Folder/SimpleBool 4 with a really long long title", "Change a bool.", &g_testdata.simplebool_target);

	debuginator_new_folder_item(debuginator, NULL, "Folder 2", 0);

	static const char* string_values[3] = { "gamestring 1", "gamestring 2", "gamestring 3"};
	static const char* string_titles[3] = { "First value", "Second one", "This is the third." };
	debuginator_create_array_item(debuginator, NULL, "Folder 2/String item",
		"Do it", unittest_on_item_changed_stringtest, &g_testdata,
		string_titles, (void*)string_values, 3, sizeof(string_values[0]));

}

static void unittest_debug_menu_run() {
	memset(&g_testdata, 0, sizeof(g_testdata));
	UnitTestData* testdata = &g_testdata;
	//DebuginatorItem item_buffer[16];
	TheDebuginatorConfig config;
	debuginator_get_default_config(&config);
	//config.item_buffer = item_buffer;
	//config.item_buffer_capacity = sizeof(item_buffer) / sizeof(item_buffer[0]);
	config.draw_rect = draw_rect;
	config.draw_text = draw_text;
	config.app_user_data = NULL;
	//config.word_wrap = word_wrap; TODO FIX

	config.size.x = 500;
	config.size.y = 1000;
	config.focus_height = 0.6f;

	TheDebuginator debuginator;
	debuginator_create(&config, &debuginator);
	unittest_debug_menu_setup(&debuginator);

	printf("\n");
	printf("Setup errors found: %u/%u\n", 
		testdata->error_index, testdata->num_tests);

	if (testdata->error_index > 0) {
		printf("Errors found during setup, exiting.\n");
		return;
	}

	testdata->num_tests = 0;

	{
		// Are our expectations after setup correct?
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "SimpleBool 1", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
		ASSERT(expected_hot_item->leaf.is_active == false);

		ASSERT(testdata->simplebool_target == false);
	}
	{
		// Nothing changes if nothing happens
		DebuginatorInput input = {0};
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == false);
	}
	{
		// Going to child activates SimpleBool 1
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(debuginator.hot_item->leaf.is_active == true);
	}
	{
		// Going to child changes SimpleBool 1 bool
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == false);
	}
	{
		// Going to child and sibling at the same time changes SimpleBool 1's to second option and sets bool to true
		DebuginatorInput input = {0};
		input.move_to_child = true;
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == true);
	}
	{
		// Going to child SimpleBool 1's first option changes bool to false
		DebuginatorInput input = {0};
		input.move_to_child = true;
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == false);
	}
	{
		// Going to parent inactivates item
		DebuginatorInput input = {0};
		input.move_to_parent = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(debuginator.hot_item->leaf.is_active == false);
	}
	{
		// Going to parent does nothing
		DebuginatorInput input = {0};
		input.move_to_parent = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "SimpleBool 1", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going down goes to Folder
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going down goes to Folder 2
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder 2", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going down wraps to SimpleBool 1
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "SimpleBool 1", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Go to Folder
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going to child goes to SimpleBool 2
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 2", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going to child activates SimpleBool 2
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(debuginator.hot_item->leaf.is_active == true);
	}
	{
		// Going to child changes SimpleBool 2 bool
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == false);
	}
	{
		// Going to parent inactivates item
		DebuginatorInput input = {0};
		input.move_to_parent = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(debuginator.hot_item->leaf.is_active == false);
	}
	{
		// Going to sibling works as expected
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 3", false);
		ASSERT(expected_hot_item == debuginator.hot_item);

		debug_menu_handle_input(&debuginator, &input);
		expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 4 with a really long long title", false);
		ASSERT(expected_hot_item == debuginator.hot_item);

		debug_menu_handle_input(&debuginator, &input);
		expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 2", false);
		ASSERT(expected_hot_item == debuginator.hot_item);

		debug_menu_handle_input(&debuginator, &input);
		expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 3", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Go to Folder 2/String item
		DebuginatorInput input = {0};
		input.move_to_parent = true;
		debug_menu_handle_input(&debuginator, &input);
		debug_menu_handle_input(&debuginator, &input);
		input.move_to_parent = false;
		input.move_sibling_next = true;
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder 2/String item", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going to child activates string item
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(debuginator.hot_item->leaf.is_active == true);
	}
	{
		// Going to child changes string
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(strcmp(testdata->stringtest, "gamestring 1") == 0);
	}
	{
		// Going down goes to next string
		DebuginatorInput input = {0};
		input.move_sibling_next = true;
		debug_menu_handle_input(&debuginator, &input);
		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder 2/String item", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Going to child changes string
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(strcmp(testdata->stringtest, "gamestring 2") == 0);
	}
	{
		// Overwrite the item with another item
		debuginator_create_bool_item(&debuginator, "Folder 2/String item", "Change a bool.", &testdata->simplebool_target);
	}
	{
		// Activate it, we should still be on the second value, so bool should turn to true
		ASSERT(testdata->simplebool_target == false);
		DebuginatorInput input = {0};
		input.move_to_child = true;
		debug_menu_handle_input(&debuginator, &input);
		ASSERT(testdata->simplebool_target == true);
	}
	{
		// Remove item
		debuginator_remove_item_by_path(&debuginator, "Folder 2/String item");
		DebuginatorItem* expected_null_item = debuginator_get_item(&debuginator, NULL, "Folder 2/String item", false);
		ASSERT(expected_null_item == NULL);

		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder 2", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}
	{
		// Set hot item
		debuginator_set_hot_item(&debuginator, "Folder/SimpleBool 2");

		DebuginatorItem* expected_hot_item = debuginator_get_item(&debuginator, NULL, "Folder/SimpleBool 2", false);
		ASSERT(expected_hot_item == debuginator.hot_item);
	}

	printf("Run errors found:   %u/%u\n",
		testdata->error_index, testdata->num_tests);

	printf("\n");
	if (testdata->error_index == 0) {
		printf("No errors found, YAY!\n");
	}
	else {
		printf("U are teh sux.\n");
	}
}

int main(int argc, char **argv)
{
	(void)(argc, argv);
	unittest_debug_menu_run();

	while (true)
	{

	}
	return 0;
}
