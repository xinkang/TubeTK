/*=========================================================================

Library:   TubeTK

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

/**
 *
 * Defines macros for defining set/get functions of inputs of shadow classes
 * in TubeTK/ITKModukes that do python wrapping of TubeTK/Applications
 */

#ifndef __tubeWrappingMacros_h
#define __tubeWrappingMacros_h

/** Get input of fundamental type */
#define tubeWrapGetMacro( name, type, wrap_filter_object_name )   \
  type Get##name( void ) const                                    \
    {                                                             \
    return this->m_##wrap_filter_object_name->Get##name();        \
    }

/** Get pointer to input of object type. */
#define tubeWrapGetObjectMacro( name, type, wrap_filter_object_name )    \
  type * Get##name( void )                                               \
    {                                                                    \
    return this->m_##wrap_filter_object_name->Get##name();               \
    }

/** Get a const pointer to input of object type. */
#define tubeWrapGetConstObjectMacro( name, type, wrap_filter_object_name )   \
  const type * Get##name( void ) const                                       \
    {                                                                        \
    return this->m_##wrap_filter_object_name->Get##name();                   \
    }

/** Set input of fundamental type */
#define tubeWrapSetMacro( name, type, wrap_filter_object_name )   \
  void Set##name( const type value )                              \
    {                                                             \
    if( this->m_##wrap_filter_object_name->Get##name() != value ) \
      {                                                           \
      this->m_##wrap_filter_object_name->Set##name( value );      \
      this->Modified();                                           \
      }                                                           \
    }

/** Set input of fundamental type */
#define tubeWrapSetObjectMacro( name, type, wrap_filter_object_name )   \
  void Set##name( type * value )                                        \
    {                                                                   \
    if( this->m_##wrap_filter_object_name->Get##name() != value )       \
      {                                                                 \
      this->m_##wrap_filter_object_name->Set##name( value );            \
      this->Modified();                                                 \
      }                                                                 \
    }

/** Set input of fundamental type */
#define tubeWrapSetConstObjectMacro( name, type, wrap_filter_object_name )   \
  void Set##name( const type * value )                                       \
    {                                                                        \
    if( this->m_##wrap_filter_object_name->Get##name() != value )            \
      {                                                                      \
      this->m_##wrap_filter_object_name->Set##name( value );                 \
      this->Modified();                                                      \
      }                                                                      \
    }

/** Proxy GetMTime of wrapped filter where all the logic resides */
#define tubeWrapUpdateMacro( wrap_filter_object_name )                \
  void Update()                                                       \
    {                                                                 \
    this->m_##wrap_filter_object_name->Update();                      \
    }                                                                 \

#endif
