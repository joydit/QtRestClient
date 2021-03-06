#include "objectbuilder.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QVersionNumber>

ObjectBuilder::ObjectBuilder(QXmlStreamReader &inStream) :
	RestBuilder(inStream)
{}

bool ObjectBuilder::canReadType(const QString &type)
{
	return type == QStringLiteral("RestObject") ||
			type == QStringLiteral("RestGadget");
}

void ObjectBuilder::build()
{
	readData();
	if(data.isObject)
		generateApiObject();
	else
		generateApiGadget();
}

void ObjectBuilder::readData()
{
	if(reader.name() == QStringLiteral("RestObject"))
		data.isObject = true;
	else if(reader.name() == QStringLiteral("RestGadget"))
		data.isObject = false;
	else
		Q_UNREACHABLE();

	data.name = readAttrib(QStringLiteral("name"), {}, true);
	data.base = readAttrib(QStringLiteral("base"), data.isObject ? QStringLiteral("QObject") : QString{});
	data.exportKey = readAttrib(QStringLiteral("export"));
	data.nspace = readAttrib(QStringLiteral("namespace"));
	data.qmlUri = readAttrib(QStringLiteral("qmlUri"));
	data.registerConverters = readAttrib<bool>(QStringLiteral("registerConverters"), true);
	data.testEquality = readAttrib<bool>(QStringLiteral("testEquality"), true);
	data.generateEquals = readAttrib<bool>(QStringLiteral("generateEquals"), !data.isObject);
	data.generateReset = readAttrib<bool>(QStringLiteral("generateReset"), true);
	data.simpleHref = readAttrib(QStringLiteral("simpleHref"));

	data.includes = {
		{false, QStringLiteral("QtCore/QObject")},
		{false, QStringLiteral("QtCore/QString")},
		{false, data.isObject ? QStringLiteral("QtCore/QScopedPointer") : QStringLiteral("QtCore/QSharedData")}
	};

	while(reader.readNextStartElement()) {
		checkError();
		if(reader.name() == QStringLiteral("Include"))
			data.includes.append(readInclude());
		else if(reader.name() == QStringLiteral("Enum"))
			data.enums.append(readEnum());
		else if(reader.name() == QStringLiteral("Property"))
			data.properties.append(readProperty());
		else
			throwChild();
	}
	checkError();
}

ObjectBuilder::XmlContent::Enum ObjectBuilder::readEnum()
{
	XmlContent::Enum enumElement;
	enumElement.name = readAttrib(QStringLiteral("name"), {}, true);
	enumElement.base = readAttrib(QStringLiteral("base"));
	enumElement.isFlags = readAttrib<bool>(QStringLiteral("isFlags"), false);

	while(reader.readNextStartElement()) {
		checkError();
		auto name = readAttrib(QStringLiteral("name"));
		enumElement.keys.append({name, reader.readElementText()});
		checkError();
	}
	return enumElement;
}

ObjectBuilder::XmlContent::Property ObjectBuilder::readProperty()
{
	auto generateReset = readAttrib<bool>(QStringLiteral("generateReset"), data.generateReset);
	XmlContent::Property prop = readBaseParam();
	prop.generateReset = generateReset;
	return prop;
}

bool ObjectBuilder::hasNs()
{
	return !data.nspace.isEmpty();
}

bool ObjectBuilder::hasQml()
{
	return !data.qmlUri.isEmpty();
}

void ObjectBuilder::generateApiObject()
{
	//write header
	writeIncludes(data.includes);
	if(hasNs())
		header << "namespace " << data.nspace << " {\n\n";
	header << "class " << data.name << "Private;\n"
		   << "class " << exportedName(data.name, data.exportKey) << " : public " << data.base << "\n"
		   << "{\n"
		   << "\tQ_OBJECT\n\n";
	writeProperties();
	header << "\npublic:\n";
	if(!data.enums.isEmpty())
		writeEnums();
	header << "\tQ_INVOKABLE " << data.name << "(QObject *parent = nullptr);\n"
		   << "\t~User() override;\n\n";
	writeReadDeclarations();
	if(data.generateEquals)
		writeEqualsDeclaration();
	header << "\npublic Q_SLOTS:\n";
	writeWriteDeclarations();
	writeResetDeclarations();

	header << "\nQ_SIGNALS:\n";
	for(const auto &prop : qAsConst(data.properties))
		header << "\tvoid " << prop.key << "Changed(const " << prop.type << " &" << prop.key << ");\n";
	header << "\nprivate:\n"
		   << "\tQScopedPointer<" << data.name << "Private> d;\n"
		   << "};\n\n";
	if(hasNs())
		header << "}\n\n";
	writeFlagOperators();

	//write source
	writeSourceIncludes();
	writePrivateClass();
	source << data.name << "::" << data.name << "(QObject *parent) :\n"
		   << "\t" << data.base << "{parent},\n"
		   << "\td{new " << data.name << "Private{}}\n"
		   << "{}\n\n"
		   << data.name << "::~" << data.name << "() = default;\n\n";
	writeReadDefinitions();
	if(data.generateEquals)
		writeEqualsDefinition();
	writeWriteDefinitions();
	writeResetDefinitions();
	writeSetupHooks();
}

void ObjectBuilder::generateApiGadget()
{
	//write header
	writeIncludes(data.includes);
	if(hasNs())
		header << "namespace " << data.nspace << " {\n\n";
	header << "class " << data.name << "Data;\n";
	if(data.base.isEmpty())
		header << "class " << exportedName(data.name, data.exportKey) << "\n";
	else
		header << "class " << exportedName(data.name, data.exportKey) << " : public " << data.base << "\n";
	header << "{\n"
		   << "\tQ_GADGET\n\n";
	writeProperties();
	header << "\npublic:\n";
	if(!data.enums.isEmpty())
		writeEnums();
	header << "\t" << data.name << "();\n"
		   << "\t" << data.name << "(const " << data.name << " &other);\n"
		   << "\t" << data.name << "(" << data.name << " &&other);\n"
		   << "\t~" << data.name << "();\n\n"
		   << "\t" << data.name << " &operator=(const " << data.name << " &other);\n"
		   << "\t" << data.name << " &operator=(" << data.name << " &&other);\n\n";
	writeReadDeclarations();
	header << '\n';
	writeWriteDeclarations();
	writeResetDeclarations();
	if(data.generateEquals)
		writeEqualsDeclaration();
	header << "\nprivate:\n"
		   << "\t QSharedDataPointer<" << data.name << "Data> d;\n"
		   << "};\n\n";
	if(hasNs())
		header << "}\n\n";
	header << "Q_DECLARE_METATYPE(" << nsName(data.name, data.nspace) << ")\n\n";
	writeFlagOperators();

	//write source
	writeSourceIncludes();
	writeDataClass();
	source << data.name << "::" << data.name << "() :\n";
	if(!data.base.isEmpty())
		source << "\t" << data.base << "{},\n";
	source << "\td{new " << data.name << "Data{}}\n"
		   << "{}\n\n"
		   << data.name << "::" << data.name << "(const " << data.name << " &other) = default;\n\n"
		   << data.name << "::" << data.name << "(" << data.name << " &&other) = default;\n\n"
		   << data.name << "::~" << data.name << "() = default;\n\n"
		   << data.name << " &" << data.name << "::operator=(const " << data.name << " &other) = default;\n\n"
		   << data.name << " &" << data.name << "::operator=(" << data.name << " &&other) = default;\n";
	writeReadDefinitions();
	writeWriteDefinitions();
	writeResetDefinitions();
	if(data.generateEquals)
		writeEqualsDefinition();
	writeSetupHooks();
}

QString ObjectBuilder::setter(const QString &name)
{
	return QStringLiteral("set") + name.mid(0, 1).toUpper() + name.mid(1);
}

void ObjectBuilder::writeEnums()
{
	for(const auto &eElem : qAsConst(data.enums)) {
		if(!eElem.base.isEmpty())
			header << "\tenum " << eElem.name << " : " << eElem.base << " {\n";
		else
			header << "\tenum " << eElem.name << " {\n";
		for(const auto &value : eElem.keys) {
			if(value.value.isEmpty())
				header << "\t\t" << value.key << ",\n";
			else
				header << "\t\t" << value.key << " = " << value.value << ",\n";
		}
		header << "\t};\n";

		if(eElem.isFlags) {
			header << "\tQ_DECLARE_FLAGS(" << eElem.name << "s, " << eElem.name << ")\n"
				   << "\tQ_FLAG(" << eElem.name << "s)\n\n";
		} else
			header << "\tQ_ENUM(" << eElem.name << ")\n\n";
	}
}

void ObjectBuilder::writeFlagOperators()
{
	auto hasFlags = false;
	for(const auto &eElem : qAsConst(data.enums)) {
		if(eElem.isFlags){
			hasFlags = true;
			header << "Q_DECLARE_OPERATORS_FOR_FLAGS(" << nsName(data.name, data.nspace) << "::" << eElem.name << "s)\n";
		}
	}

	if(hasFlags)
		header << "\n";
}

void ObjectBuilder::writeProperties()
{
	for(const auto &prop : qAsConst(data.properties)) {
		header << "\tQ_PROPERTY(" << prop.type << " " << prop.key
			   << " READ " << prop.key
			   << " WRITE " << setter(prop.key);
		if(prop.generateReset)
			header << " RESET re" << setter(prop.key);
		if(data.isObject)
			header << " NOTIFY " << prop.key << "Changed";
		header << ")\n";
	}
}

void ObjectBuilder::writeReadDeclarations()
{
	for(const auto &prop : qAsConst(data.properties))
		header << "\t" << prop.type << " " << prop.key << "() const;\n";
	if(!data.simpleHref.isEmpty())
		header << "\n\tQUrl extensionHref() const override;\n";
}

void ObjectBuilder::writeWriteDeclarations()
{
	for(const auto &prop : qAsConst(data.properties))
		header << "\tvoid " << setter(prop.key) << "(" << prop.type << " " << prop.key << ");\n";
}

void ObjectBuilder::writeResetDeclarations()
{
	auto once = true;
	for(const auto &prop : qAsConst(data.properties)) {
		if(!prop.generateReset)
			continue;
		if(once) {
			once = false;
			header << '\n';
		}
		header << "\tvoid re" << setter(prop.key) << "();\n";
	}
}

void ObjectBuilder::writeMemberDeclarations()
{
	for(const auto &prop : qAsConst(data.properties))
		source << "\t" << prop.type << " " << prop.key << ";\n";
}

void ObjectBuilder::writeEqualsDeclaration()
{
	if(data.isObject) {
		header << "\n\tQ_INVOKABLE bool equals(const " << data.name << " *other) const;\n";
	} else {
		header << "\n\tbool operator==(const " << data.name << " &other) const;\n";
		header << "\tbool operator!=(const " << data.name << " &other) const;\n";
	}
}

void ObjectBuilder::writeSourceIncludes()
{
	source << "#include \"" << fileName << ".h\"\n\n"
		   << "#include <QtCore/QVariant>\n";
	if(data.registerConverters) {
		source << "#include <QtCore/QCoreApplication>\n"
			   << "#include <QtJsonSerializer/QJsonSerializer>\n";
	}
	if(hasQml())
		source << "#include <QtQml/qqml.h>\n";
	if(hasNs())
		source << "using namespace " << data.nspace << ";\n";
	source << '\n';
}

void ObjectBuilder::writeReadDefinitions()
{
	for(const auto &prop : qAsConst(data.properties)) {
		source << "\n" << prop.type << " " << data.name << "::" << prop.key << "() const\n"
			   << "{\n"
			   << "\treturn d->" << prop.key << ";\n"
			   << "}\n";
	}
	if(!data.simpleHref.isEmpty()) {
		source << "\nQUrl " << data.name << "::extensionHref() const\n"
			   << "{\n"
			   << "\treturn d->" << data.simpleHref << ";\n"
			   << "}\n";
	}
}

void ObjectBuilder::writeWriteDefinitions()
{
	for(const auto &prop : qAsConst(data.properties)) {
		source << "\nvoid " << data.name << "::" << setter(prop.key) << "(" << prop.type << " " << prop.key << ")\n"
			   << "{\n";
		if(data.testEquality) {
			source << "\tif(d->" << prop.key << " == " << prop.key << ")\n"
				   << "\t\treturn;\n\n";
		}
		source << "\td->" << prop.key << " = std::move(" << prop.key <<");\n";
		if(data.isObject)
			source << "\temit " << prop.key << "Changed(d->" << prop.key << ");\n";
		source << "}\n";
	}
}

void ObjectBuilder::writeResetDefinitions()
{
	for(const auto &prop : qAsConst(data.properties)) {
		if(!prop.generateReset)
			continue;

		source << "\nvoid " << data.name << "::re" << setter(prop.key) << "()\n"
			   << "{\n"
			   << "\t" << setter(prop.key) << "(";
		if(prop.defaultValue.isEmpty())
			source << prop.type << "{}";
		else
			source << writeParamDefault(prop);
		source << ");\n"
			   << "}\n";
	}
}

void ObjectBuilder::writeEqualsDefinition()
{
	QString otherPrefix = (data.isObject ? QStringLiteral("->") : QStringLiteral(".")) + QStringLiteral("d->");
	//equals
	if(data.isObject)
		source << "\nbool " << data.name << "::" << "equals(const " << data.name << " *other) const\n";
	else
		source << "\nbool " << data.name << "::" << "operator==(const " << data.name << " &other) const\n";
	source << "{\n"
		   << "\treturn true";
	for(const auto &prop : qAsConst(data.properties))
		source << "\n\t\t&& d->" << prop.key << " == other" << otherPrefix << prop.key;
	source << ";\n"
		   << "}\n";

	//unequals
	if(!data.isObject) {
		source << "\nbool " << data.name << "::" << "operator!=(const " << data.name << " &other) const\n"
			   << "{\n"
			   << "\treturn false";
		for(const auto &prop : qAsConst(data.properties))
			source << "\n\t\t|| d->" << prop.key << " != other" << otherPrefix << prop.key;
		source << ";\n"
			   << "}\n";
	}
}

void ObjectBuilder::writePrivateClass()
{
	QString name = data.name + QStringLiteral("Private");
	if(hasNs())
		source << "namespace " << data.nspace << " {\n\n";
	source << "class " << name << "\n"
		   << "{\n"
		   << "public:\n"
		   << "\t" << name << "() :\n";
	writeMemberDefinitions(true);
	source << "\t{}\n\n";
	writeMemberDeclarations();
	source << "};\n\n";
	if(hasNs())
		source << "}\n\n";
}

void ObjectBuilder::writeDataClass()
{
	QString name = data.name + QStringLiteral("Data");
	if(hasNs())
		source << "namespace " << data.nspace << " {\n\n";
	source << "class " << name << " : public QSharedData\n"
		   << "{\n"
		   << "public:\n"
		   << "\t" << name << "() :\n"
		   << "\t\tQSharedData{}\n";
	writeMemberDefinitions(false);
	source << "\t{}\n\n"
		   << "\t" << name << "(const " << name << " &other) = default;\n"
		   << "\t" << name << "(" << name << " &&other) = default;\n\n";
	writeMemberDeclarations();
	source << "};\n\n";
	if(hasNs())
		source << "}\n\n";
}

void ObjectBuilder::writeMemberDefinitions(bool skipComma)
{
	for(const auto &prop : qAsConst(data.properties)) {
		if(skipComma) {
			source << "\t\t" << prop.key << "{";
			skipComma = false;
		} else
			source << "\t\t," << prop.key << "{";
		if(!prop.defaultValue.isEmpty())
			source << writeParamDefault(prop);
		source << "}\n";
	}
}

void ObjectBuilder::writeSetupHooks()
{
	if(!data.registerConverters && !hasQml())
		return;

	source << "\nnamespace {\n\n"
		   << "void __" << data.name << "_setup_hook()\n"
		   << "{\n";

	if(data.registerConverters)
		source << "\tQJsonSerializer::registerListConverters<" << data.name << (data.isObject ? "*" : "") << ">();\n";
	if(hasQml()) {
		auto uriParts = data.qmlUri.split(QLatin1Char(' '), QString::SkipEmptyParts);
		auto uriPath = uriParts.takeFirst();
		auto uriVersion = QVersionNumber::fromString(uriParts.join(QLatin1Char(' ')));
		if(uriVersion.isNull())
			uriVersion = {1,0};
		if(data.isObject) {
			source << "\tqmlRegisterType<" << data.name << ">(\""
				   << uriPath << "\", " << uriVersion.majorVersion() << ", " << uriVersion.minorVersion()
				   << ", \"" << data.name << "\");\n";
		} else {
			source << "\tqmlRegisterUncreatableType<" << data.name << ">(\""
				   << uriPath << "\", " << uriVersion.majorVersion() << ", " << uriVersion.minorVersion()
				   << ", \"" << data.name << "\", QStringLiteral(\"Q_GADGETs cannot be created from QML\"));\n";
		}
	}

	source << "}\n\n"
		   << "}\n"
		   << "Q_COREAPP_STARTUP_FUNCTION(__" << data.name << "_setup_hook)\n";
}
