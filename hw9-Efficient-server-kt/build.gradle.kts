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
    mainClass.set("necasond.MainKt")
}

dependencies {
    implementation(kotlin("stdlib"))

    implementation("com.google.protobuf:protobuf-java:4.27.2")

    // Coroutines for asynchronous programming
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.8.1")

    // Serialization library
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-core:1.7.1")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-protobuf:1.7.1")

    testImplementation(kotlin("test"))
}

tasks.test {
    useJUnitPlatform()
}

kotlin {
    jvmToolchain(21)
}
