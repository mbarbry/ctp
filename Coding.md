# Coding standards #

## files ##
  * each class goes into a separate file
  * filename is name of class in lowercase

## braces ##
  * in functions/classes, the { is in the next line
  * for for loops, if, ..., the { is n the same line as if,for

## classes ##
  * normally begin in upper case
  * exceptions: classes which define "small" types (e.g. `vec`, `matrix`)
  * order in class definition:
    * first `public` all functions
    * then `private`/`protected` all member variables
    * then `private`/`protected` member functions
    * no rule where to define a `public typedef` in the class
  * all member variables are `private`/`public` (again exception e.g. `vec`, `matrix`)
  * maximum one-line-function implementation in class declaration, everything else moves to separate file or inline at end of header.

## naming in classes ##
  * all member variables are in lower case and start with `_`
  * all functions start with upper case, no `_` in names
  * exception: `get`/`set` functions

## get/set functions ##
  * get/set functions start with a lowercase set/get (these are only functions which directly set/get a private member variable)
  * get must return a constant reference and keep the `class const`: `const int &getId() const;`
  * set only sets the member, e.g. `void setId(const int &id) {_id=id;}`