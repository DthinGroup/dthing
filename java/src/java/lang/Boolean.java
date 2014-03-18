/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.lang;

/**
 * The wrapper for the primitive type {@code boolean}.
 *
 * @since 1.0
 */
public final class Boolean {

    private static final long serialVersionUID = -3665804199014368530L;

    /**
     * The boolean value of the receiver.
     */
    private final boolean value;

    /**
     * The {@code Boolean} object that represents the primitive value
     * {@code true}.
     */
    public static final Boolean TRUE = new Boolean(true);

    /**
     * The {@code Boolean} object that represents the primitive value
     * {@code false}.
     */
    public static final Boolean FALSE = new Boolean(false);

    /**
     * Constructs a new {@code Boolean} with the specified primitive boolean
     * value.
     *
     * @param value
     *            the primitive boolean value, {@code true} or {@code false}.
     */
    public Boolean(boolean value) {
        this.value = value;
    }

    /**
     * Gets the primitive value of this boolean, either {@code true} or
     * {@code false}.
     *
     * @return this object's primitive value, {@code true} or {@code false}.
     */
    public boolean booleanValue() {
        return value;
    }

    /**
     * Compares this instance with the specified object and indicates if they
     * are equal. In order to be equal, {@code o} must be an instance of
     * {@code Boolean} and have the same boolean value as this object.
     *
     * @param o
     *            the object to compare this boolean with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Boolean}; {@code false} otherwise.
     */
    @Override
    public boolean equals(Object o) {
        return (o == this) || ((o instanceof Boolean) && (((Boolean) o).value == value));
    }

    /**
     * Returns an integer hash code for this boolean.
     *
     * @return this boolean's hash code, which is {@code 1231} for {@code true}
     *         values and {@code 1237} for {@code false} values.
     */
    @Override
    public int hashCode() {
        return value ? 1231 : 1237;
    }

    /**
     * Returns a string containing a concise, human-readable description of this
     * boolean.
     *
     * @return "true" if the value of this boolean is {@code true}, "false"
     *         otherwise.
     */
    @Override
    public String toString() {
        return String.valueOf(value);
    }

    /**
     * Converts the specified boolean to its string representation.
     *
     * @param value
     *            the boolean to convert.
     * @return "true" if {@code value} is {@code true}, "false" otherwise.
     */
    public static String toString(boolean value) {
        return String.valueOf(value);
    }
}
