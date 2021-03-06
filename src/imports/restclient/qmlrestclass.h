#ifndef QTRESTCLIENT_QMLRESTCLASS_H
#define QTRESTCLIENT_QMLRESTCLASS_H

#include <tuple>
#include <QtCore/QObject>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlListProperty>
#include <QtQml/QJSValue>

#include <QtRestClient/RestClass>

#ifdef DOXYGEN_RUN
namespace de::skycoder42::QtRestClient {

/*! @brief The QML version of ::QtRestClient::RestClass
 *
 * @extends QtQml.QtObject
 * @since 2.0
 */
class RestClass
{
#else
namespace QtRestClient {

class QmlRestClass : public QObject, public QQmlParserStatus
{
	Q_OBJECT
	Q_INTERFACES(QQmlParserStatus)
#endif

	/*! @brief The ::QtRestClient::RestClass made available via the QML variant
	 *
	 * @default{<i>auto</i>}
	 * @accessors{
	 *	@memberAc{restClass}
	 *	@memberAc{restClassChanged}
	 * }
	 * @sa ::QtRestClient::RestClass
	 */
	Q_PROPERTY(QtRestClient::RestClass* restClass READ restClass NOTIFY restClassChanged)
	/*! @brief The subpath to the parent class/api this class adds
	 *
	 * @default{<i>empty</i>}
	 * @accessors{
	 *	@memberAc{path}
	 *	@notifyAc{pathChanged()}
	 * }
	 */
	Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
#ifdef DOXYGEN_RUN
	/*! @brief A helper property to declare RestClasses for the class as subclasses
	 *
	 * @default{<i>empty</i>}
	 * @accessors{
	 *	@memberAc{classes}
	 * }
	 */
	Q_PROPERTY(QQmlListProperty<de::skycoder42::QtRestClient::RestClass> classes READ classes)
#else
	Q_PROPERTY(QQmlListProperty<QtRestClient::QmlRestClass> classes READ classes)
#endif

	Q_CLASSINFO("DefaultProperty", "classes")

public:
	//! Default constructor with object parent
	explicit QmlRestClass(QObject *parent = nullptr);

	//! @private
	RestClass* restClass() const;
	//! @private
	QString path() const;
	//! @private
	QQmlListProperty<QmlRestClass> classes();

	//! @private
	void classBegin() override;
	//! @private
	void componentComplete() override;

	/*! @brief Perform a HTTP-Request call on this class
	 *
	 * @param verb The HTTP-Verb to be used
	 * @param optBody Optional and variable parameter
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optAsPost Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [body:object|array,] [path:string,] [params:object, [asPost:bool], [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::call
	 */
	Q_INVOKABLE QtRestClient::RestReply *call(const QString &verb,
											  const QJSValue &optBody = {},
											  const QJSValue &optPath = {},
											  const QJSValue &optParams = {},
											  const QJSValue &optAsPost = {},
											  const QJSValue &optHeaders = {});

	/*! @brief Perform a HTTP-GET-Request on this class
	 *
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::get
	 */
	Q_INVOKABLE QtRestClient::RestReply *get(const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});
	/*! @brief Perform a HTTP-POST-Request on this class
	 *
	 * @param optBody Optional and variable parameter
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [body:object|array,] [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::post
	 */
	Q_INVOKABLE QtRestClient::RestReply *post(const QJSValue &optBody = {}, const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});
	/*! @brief Perform a HTTP-PUT-Request on this class
	 *
	 * @param body The data body of the HTTP-Request, sent as json (can be `object` or `array`)
	 * @param optBody Optional and variable parameter
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [body:object|array,] [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::put
	 */
	Q_INVOKABLE QtRestClient::RestReply *put(const QJSValue &body, const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});
	// [path,] [params, [headers]]
	/*! @brief Perform a HTTP-DELETE-Request on this class
	 *
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::deleteResource
	 */
	Q_INVOKABLE QtRestClient::RestReply *deleteResource(const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});
	// body, [path,] [params, [headers]]
	/*! @brief Perform a HTTP-PATCH-Request on this class
	 *
	 * @param body The data body of the HTTP-Request, sent as json (can be `object` or `array`)
	 * @param optBody Optional and variable parameter
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [body:object|array,] [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::patch
	 */
	Q_INVOKABLE QtRestClient::RestReply *patch(const QJSValue &body, const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});
	// [path,] [params, [headers]]
	/*! @brief Perform a HTTP-HEAD-Request on this class
	 *
	 * @param optPath Optional and variable parameter
	 * @param optParams Optional and variable parameter
	 * @param optHeaders Optional and variable parameter
	 *
	 * The method parameters are declared as var to be able to provide advance syntax.
	 * They are all placeholders for different types and can be used with the following syntax
	 * (the format is `<name>:<type>[|<type>...]` for every possible param):
	 * @code
	 * [path:string,] [params:object, [headers:object]]
	 * @endcode
	 *
	 * @sa ::QtRestClient::RestClass::head
	 */
	Q_INVOKABLE QtRestClient::RestReply *head(const QJSValue &optPath = {}, const QJSValue &optParams = {}, const QJSValue &optHeaders = {});

public Q_SLOTS:
	//! @private
	void setPath(QString path);

Q_SIGNALS:
	//! @notifyAcFn{RestClass::restClass}
	void restClassChanged(QtRestClient::RestClass *restClass);
	//! @notifyAcFn{RestClass::path}
	void pathChanged(QString path);

private Q_SLOTS:
	void revaluateClass();

private:
	RestClass *_class = nullptr;
	QString _path;
	QList<QmlRestClass*> _childClasses;
	bool _init = false;

	// [body,] [path,] [params, [asPost], [headers]]
	RestReply *callImpl(const QByteArray &verb, const QJSValue &arg0 = {}, const QJSValue &arg1 = {}, const QJSValue &arg2 = {}, const QJSValue &arg3 = {}, const QJSValue &arg4 = {});
	RestReply *callImpl2(const QByteArray &verb, bool forcePost, const QJSValue &arg0 = {}, const QJSValue &arg1 = {}, const QJSValue &arg2 = {}, const QJSValue &arg3 = {}, const QJSValue &arg4 = {});
};

}

#endif // QTRESTCLIENT_QMLRESTCLASS_H
