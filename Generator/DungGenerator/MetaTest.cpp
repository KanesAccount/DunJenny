#include "MetaTest.h"

std::vector<std::string> MetaTest::PrintMetaTest()
{
	Person person;
	person.age = 25;
	person.salary = 3.50f;
	person.name = "Alex"; // I'm a person!

	person.favouriteMovies["Nostalgia Critic"] = { MovieInfo{ "The Room", 8.5f } };
	person.favouriteMovies["John Tron"] = { MovieInfo{ "Goosebumps", 10.0f },
		MovieInfo{ "Talking Cat", 9.0f } };

	// printing members of different classes
	//std::cout << "Members of class Person:\n";
	meta::doForAllMembers<Person>(
		[](const auto& member)
	{
		//strings.push_back(member.getName());
		//std::cout << "* " << member.getName() << '\n';
	}
	);

	strings.push_back("Members of class MovieInfo:\n");
	//std::cout << "Members of class MovieInfo:\n";
	meta::doForAllMembers<MovieInfo>(
		[](const auto& member)
	{
		//strings.push_back(member.getName());
		//std::cout << "* " << member.getName() << '\n';
	}
	);

	// checking if classes are registered
	if (meta::isRegistered<Person>()) {
		strings.push_back("Person class is registered\n");
		std::string s = "It has " + meta::getMemberCount<Person>();
		strings.push_back(s);
		//std::cout << "Person class is registered\n";
		//std::cout << "It has " << meta::getMemberCount<Person>() << " members registered.\n";
	}

	// meta::isRegistered is constexpr, so can be used in enable_if and static_assert!
	static_assert(meta::isRegistered<Person>(), "Person class is not registered!");
	//static_assert(meta::getMemberCount<Person>() == 4, "Person does not have 4 members registered?");

	if (!meta::isRegistered<Unregistered>()) {
		strings.push_back("Unregistered class is unregistered\n It has ");
		std::string s = meta::getMemberCount<Unregistered>() + " members registered.\n";
		//std::cout << "Unregistered class is unregistered\n";
		//std::cout << "It has " << meta::getMemberCount<Unregistered>() << " members registered.\n";
	}

	// checking if class has a member
	if (meta::hasMember<Person>("age")) {
		strings.push_back("Person has member named 'age'\n");
		//std::cout << "Person has member named 'age'\n";
	}

	// getting setting member values
	auto name = meta::getMemberValue<std::string>(person, "name");
	strings.push_back("Got person's name: " + name + '\n');
	//std::cout << "Got person's name: " << name << '\n';

	meta::setMemberValue<std::string>(person, "name", "Ron Burgundy");
	name = meta::getMemberValue<std::string>(person, "name");
	strings.push_back("Changed person's name to " + name + '\n');
	//std::cout << "Changed person's name to " << name << '\n';


	// And here's how you can serialize/deserialize
	// (if you write a function for your type)
	strings.push_back("Serializing person:" + '\n');
	//std::cout << "Serializing person:" << '\n';

	json root = person;
	//std::string r = (std::setw(4), root);
	//strings.push_back(root);
	///std::cout << std::setw(4) << root << std::endl;

	//Unregistered y;
	//json root2 = y; // this will fail at compile time

	strings.push_back("Serializing Person 2 from JSON:\n");
	//std::cout << "Serializing Person 2 from JSON:\n";

	auto person2 = root.get<Person>();
	strings.push_back("Person 2 name is " + person2.getName() + " too!" + '\n');
	//std::cout << "Person 2 name is " << person2.getName() << " too!" << '\n';
	return strings;
}


