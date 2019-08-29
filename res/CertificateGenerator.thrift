#!/usr/local/bin/thrift --gen cpp

namespace cpp CertificateGeneratorThrift

struct File {
    1: required string name;
    2: required binary content;
}

exception InvalidConfiguration {
1: string message,
}

exception InvalidTemplate {
1: string message,
}

exception InvalidRessource {
1: string message,
}

exception InternalServerError {
1: string message,
}

service CertificateGenerator {
  void setConfigurationData(1:string configuration),
  void addResourceFile(1:File resourceFile),
  void addTemplateFile(1:File templateFile),
  void addResourceFiles(1:list<File> resourceFiles),
  void addTemplateFiles(1:list<File> templateFiles),
  bool checkJob(),
  list<File> generateCertificates(),
}
