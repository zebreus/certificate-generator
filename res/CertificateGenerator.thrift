#!/usr/local/bin/thrift --gen cpp

namespace cpp CertificateGeneratorThrift

struct GeneratedFile {
    1: required string name;
    2: required binary content;
}

exception InvalidConfiguration {
1: string message,
}

exception InvalidTemplate {
1: string message,
}

exception UnableToWriteFile {
1: string message,
}

service CertificateGenerator {
  list<GeneratedFile> generateCertificates(1:string configuration),
}
