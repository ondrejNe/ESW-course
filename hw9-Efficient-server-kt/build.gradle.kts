plugins {
    id("java")
    id("idea")
    application
    kotlin("jvm") version "1.9.23"
}

group = "necasond"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
    gradlePluginPortal()
}

application {
    mainClass.set("necasond.Application")
}

dependencies {
    // Protobuf
    implementation(libs.protobuf.java)

    // Logging
    implementation(libs.slf4j.api)
    implementation(libs.logback.classic)
    implementation(libs.logstash.logback.encoder)

    // Coroutines for asynchronous programming
    implementation(libs.coroutines.core)

    // Serialization library
    implementation(libs.serialization.core)
    implementation(libs.serialization.protobuf)
}

kotlin {
    jvmToolchain(17)
}
