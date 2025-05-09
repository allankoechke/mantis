//
// Created by allan on 09/05/2025.
//

#include <iostream>
#include "../include/mantis/core/models.h"

int main() {
    using namespace Mantis;

    BaseTable notes;
    notes.name = "notes";
    notes.fields.push_back(Field("title", FieldType::Text, true));
    notes.fields.push_back(Field("content", FieldType::Text));
    notes.fields.push_back(Field("is_archived", FieldType::Boolean));

    std::cout << "Base Table SQL:\n" << notes.to_sql() << "\n\n";

    AuthTable users;
    users.name = "users";
    std::cout << "Auth Table SQL:\n" << users.to_sql() << "\n";

    return 0;
}